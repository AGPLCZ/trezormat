import serial
import serial.tools.list_ports
import time
import threading
import json
from flask import Flask, jsonify, request
from flask_cors import CORS

###############################
# Nastavení detekce portu (původní nastavení)
###############################
USE_EXACT_VID_PID = True  # Pokud je True, hledá se podle vendor/product ID, jinak podle klíčového slova.
EXPECTED_VID = 0x0403  # FTDI
EXPECTED_PID = 0x6001  # FT232R

###############################
# 🌍 Flask API Setup
###############################
app = Flask(__name__)
CORS(app)  # Povolit přístup ze všech webových stránek
credit_file = "credit.json"  # Soubor pro uchování kreditu
ser = None  # Globální proměnná pro sériový port
escrow_value = 0  # Bankovka aktuálně v escrow

# 🔥 Proměnné pro správu kreditu
credit = 0              # Zbývající herní čas v sekundách
total_inserted = 0      # Celková částka (v MIL PYG), kterou uživatel vložil

###############################
# ⏳ Nastavení času za bankovku
###############################
# Kolik sekund hraní dostane hráč za 1 MIL PYG
TIME_PER_MIL = 120  # 1 MIL PYG = 120 sekund (2 minuty); 5 MIL PYG = 10 minut

###############################
# 💾 Funkce pro ukládání a načítání kreditu
###############################
def save_credit():
    """Uloží kredit a celkovou vloženou částku do souboru JSON."""
    with open(credit_file, "w") as f:
        json.dump({"credit": credit, "total_inserted": total_inserted}, f)

def load_credit():
    """Načte kredit a celkovou vloženou částku ze souboru JSON."""
    global credit, total_inserted
    try:
        with open(credit_file, "r") as f:
            data = json.load(f)
            credit = data.get("credit", 0)
            total_inserted = data.get("total_inserted", 0)
    except FileNotFoundError:
        credit = 0
        total_inserted = 0

###############################
# 🌍 Flask API pro získání kreditu
###############################
@app.route("/api/credit")
def get_credit():
    return jsonify({"credit": credit, "total_inserted": total_inserted})

###############################
# Endpoint pro reset kreditu (volitelný)
###############################
@app.route("/api/reset_credit", methods=["POST"])
def reset_credit_api():
    reset_credit()
    return jsonify({"message": "Kredit a celková vložená částka byly resetovány."})

###############################
# 💰 Správa kreditu a časového odpočtu
###############################
def bill(value):
    """
    Přičte hodnotu bankovky ke kreditu a aktualizuje celkovou vloženou částku.
    Nová hodnota se přičítá ke stávajícímu času.
    """
    global credit, total_inserted
    time_added = value * TIME_PER_MIL  # Vypočítáme herní čas za vloženou hodnotu
    credit += time_added               # Přičteme nový čas k aktuálnímu
    total_inserted += value            # Akumulujeme celkovou vloženou hodnotu
    print(f"💰 Kredit přidán: {value} MIL -> +{time_added} sekund. Celkem: {credit} sekund, Celkem vloženo: {total_inserted} MIL")
    save_credit()

def start_credit_countdown():
    """
    Odečítá kredit každou sekundu. Když kredit dosáhne nuly,
    jednou vypíše hlášení o vypršení času a čeká na nový vklad.
    """
    global credit
    timeout_printed = False  # Příznak, zda již bylo oznámeno, že čas vypršel
    while True:
        if credit > 0:
            credit -= 1
            timeout_printed = False  # Resetujeme příznak, jakmile je kredit > 0
            print(f"⏳ Zbývající kredit: {credit} sekund")
            save_credit()
        else:
            # Pokud kredit = 0, jednorázově vypíšeme hlášení a poté nebudeme hlásit opakovaně
            if not timeout_printed:
                print("❌ Čas vypršel, čekám na vložení další bankovky...")
                timeout_printed = True
        time.sleep(1)

###############################
# 🔍 Funkce pro nalezení správného portu
###############################
def find_nv10_port(keyword="FTDI"):
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = p.description if p.description else ""
        manu = p.manufacturer if p.manufacturer else ""
        name_ = p.name if p.name else ""
        if USE_EXACT_VID_PID:
            if (p.vid == EXPECTED_VID) and (p.pid == EXPECTED_PID):
                print(f"🔍 Nalezen port podle VID/PID: {p.device}")
                return p.device
        else:
            if (keyword in desc) or (keyword in manu) or (keyword in name_):
                print(f"🔍 Nalezen port podle klíčového slova: {p.device}")
                return p.device
    return None

###############################
# 🔌 Inicializace NV10
###############################
def setup_nv10():
    global ser
    try:
        ser.write(bytes([184]))  # Acceptor enabled
        print("📤 Odesláno: Acceptor enabled (184)")

        ser.write(bytes([170]))  # Enable escrow mode
        print("📤 Odesláno: Enable serial escrow mode (170)")

        ser.write(bytes([191]))  # Verify acceptor
        print("📤 Odesláno: Verify acceptor (191)")

        print("✅ NV10 inicializován.")
        time.sleep(1)
    except Exception as e:
        print("❌ Chyba při inicializaci NV10:", e)

###############################
# 🔄 Hlavní smyčka čtení NV10
###############################
def loop_nv10():
    global credit, escrow_value
    while True:
        if ser.in_waiting > 0:
            byte_in = ser.read(1)[0]
            print(f"📥 Přijato od zařízení: {byte_in}")

            # Mapa bankovek (hodnota v MIL PYG)
            banknotes = {
                1: 2,    # 2 MIL
                2: 5,    # 5 MIL
                3: 10,   # 10 MIL
                4: 20,   # 20 MIL
                5: 50,   # 50 MIL
                6: 100   # 100 MIL
            }

            if byte_in in banknotes:
                escrow_value = banknotes[byte_in]
                print(f"💵 Bankovka {escrow_value} MIL v escrow. Odesílám ACCEPT (172).")
                ser.write(bytes([172]))  # Přijmout bankovku
                print("📤 Odesláno: ACCEPT (172)")

            elif byte_in == 70:
                print("❌ Přerušena úschova. Bankovka se vrací.")
                escrow_value = 0  # Zabrání přičtení kreditu

            elif byte_in == 172:
                if escrow_value > 0:
                    print(f"✅ Zařízení potvrdilo přijetí bankovky {escrow_value} MIL PYG.")
                    bill(escrow_value)
                    escrow_value = 0

            elif byte_in == 173:
                print("❌ Bankovka odmítnuta (173).")
                escrow_value = 0

            # Výpis dalších kódů pro úplný přehled komunikace
            elif byte_in == 184:
                print("📢 Zařízení: Acceptor enabled (184).")
            elif byte_in == 185:
                print("📢 Zařízení: Acceptor disabled (185).")
            elif byte_in == 170:
                print("📢 Zařízení: Escrow mode enabled (170).")
            elif byte_in == 191:
                print("📢 Zařízení: Verify acceptor (191).")
            elif byte_in == 182:
                print("📢 Zařízení: Status (182).")
            elif byte_in == 120:
                print("📢 Zařízení poslalo 120 (Neznámá bankovka).")
            elif byte_in == 121:
                print("📢 Zařízení poslalo 121 (neznámý význam).")
            elif byte_in == 20:
                print("📢 Zařízení poslalo 20 (neznámý význam).")
            elif byte_in == 5:
                print("📢 Zařízení poslalo 5 (Vracím).")
            else:
                print("❓ Neznámá odpověď:", byte_in)

        time.sleep(0.05)  # Snížena prodleva pro rychlejší reakci

###############################
# 🔄 Funkce pro reset kreditu a vkladu
###############################
def reset_credit():
    """
    Vynuluje kredit i celkovou vloženou částku.
    Tuto funkci lze volat například přes API a poté hra začíná znovu.
    """
    global credit, total_inserted
    credit = 0
    total_inserted = 0
    save_credit()
    print("🔄 Kredit a celková vložená částka resetovány.")

###############################
# 🚀 Spuštění programu
###############################
def main():
    global ser

    load_credit()  # Načíst kredit ze souboru
    port = find_nv10_port()
    if not port:
        print("❌ Nenalezen žádný sériový port pro NV10.")
        return

    try:
        ser = serial.Serial(port, baudrate=300, bytesize=serial.EIGHTBITS,
                            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_TWO, timeout=1)
    except Exception as e:
        print("❌ Chyba při otevírání sériového portu:", e)
        return

    setup_nv10()

    # Spustíme vlákno pro čtení NV10
    threading.Thread(target=loop_nv10, daemon=True).start()

    # Spustíme vlákno pro odpočet kreditu
    threading.Thread(target=start_credit_countdown, daemon=True).start()

    # Spustíme Flask API
    app.run(debug=True, host="0.0.0.0", port=5000)

if __name__ == "__main__":
    main()
