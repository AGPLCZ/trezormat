import serial
import serial.tools.list_ports
import time
import threading
import json
import platform
from flask import Flask, jsonify, request
from flask_cors import CORS

###############################
# Vysvětlení
###############################
# Tento skript komunikuje se zařízení NV10 (bankovkový validátor od Innovative Technology)
# prostřednictvím sériového protokolu SIO – "Serial Input/Output".
#
# Zařízení se používá v režimu ESCROW (úschova), který umožňuje, aby
# bankovka byla nejprve detekována a podržena, dokud software nerozhodne,
# zda ji má přijmout (vložit do zásobníku) nebo vrátit zpět uživateli.
#
# ---------------------------------------------------------------
# 🔧 PRINCIP KOMUNIKACE
# ---------------------------------------------------------------
# NV10 je připojen přes sériový port (např. /dev/ttyUSB0 nebo COM3)
# a pracuje s rychlostí 300 Bd (v SIO režimu, 8N2, žádná parita).
#
# Komunikace probíhá výměnou jednoduchých jednoparametrových bajtových příkazů.
# Python odesílá jednotlivé příkazy jako byte hodnoty, např.:
#   ser.write(bytes([184]))   # aktivace akceptoru
#
# NV10 pak odpovídá také jedním bytem (např. 1–16 = přijatá bankovka,
# 172 = přijetí, 173 = odmítnutí, 182 = status, apod.).
#
# ---------------------------------------------------------------
# 🧩 ZÁKLADNÍ PRINCIP ESCROW REŽIMU
# ---------------------------------------------------------------
# Escrow (úschova) je stav, kdy zařízení zachytí bankovku,
# ale fyzicky ji zatím NEvloží do zásobníku.
#
# Průběh typické transakce:
# 1️⃣ Uživatel vloží bankovku.
# 2️⃣ NV10 odešle kód kanálu (např. 1, 2, 3...) podle rozpoznané hodnoty.
# 3️⃣ Bankovka je fyzicky držena v mechanice – čeká na rozhodnutí.
# 4️⃣ Software musí odpovědět:
#      • 172 → ACCEPT – přijmout bankovku (vložit do stackeru)
#      • 173 → REJECT – odmítnout bankovku (vrátit zpět uživateli)
# 5️⃣ NV10 poté pošle potvrzení výsledku (např. 172 – přijatá, nebo 70 – přerušeno).
#
# Tímto způsobem je zaručeno, že žádná bankovka nebude započítána vícekrát,
# dokud neproběhne fyzické vložení a potvrzení.
#
# ---------------------------------------------------------------
# ⚙️ HLAVNÍ PŘÍKAZY ODESÍLANÉ DO ZAŘÍZENÍ
# ---------------------------------------------------------------
# 170 → Enable serial escrow mode (zapne escrow režim)
# 171 → Disable escrow mode
# 172 → Přijmout bankovku v úschově (ACCEPT)
# 173 → Odmítnout bankovku v úschově (REJECT)
# 182 → Vyžádání stavu (status dotaz)
# 184 → Aktivuje akceptor (Enable all channels)
# 185 → Deaktivuje akceptor (Disable all channels)
# 190 → Deaktivuje časový limit escrow
# 191 → Aktivuje časový limit escrow
# 192 → Požádá o verzi firmwaru
# 193 → Požádá o verzi datasetu (měnové rozpoznávání)
#
# ---------------------------------------------------------------
# 📩 KÓDY PŘIJÍMANÉ OD ZAŘÍZENÍ
# ---------------------------------------------------------------
# NV10 odpovídá vždy jediným bajtem, který označuje stav nebo událost:
#
#  1–16 → Rozpoznaná bankovka (kanál podle hodnoty)
#   5   → Vrácení bankovky
#  20   → Neznámá bankovka (neodpovídá žádnému kanálu)
#  30   → Mechanismus se pohybuje pomalu
#  40   → Pokus o přetažení bankovky (stringing)
#  50   → Odmítnutí podezřelé bankovky (fraudní kanál)
#  60   → Zásobník plný nebo zaseknutý
#  70   → Přerušena úschova (uživatel vytáhl bankovku)
#  80   → Bankovka byla pravděpodobně odebrána při uvolnění zaseknutí
# 120   → Validator zaneprázdněn
# 121   → Validator volný
# 170   → Escrow režim aktivován
# 171   → Escrow režim deaktivován
# 172   → Bankovka přijata
# 173   → Bankovka odmítnuta
# 182   → Status / potvrzení
# 184   → Akceptor povolen
# 185   → Akceptor zakázán
# 190   → Escrow timeout vypnut
# 191   → Escrow timeout zapnut
# 192   → Odpověď na žádost o firmware
# 193   → Odpověď na žádost o dataset
# 255   → Chyba příkazu (Command error)
#
# ---------------------------------------------------------------
# 💵 MAPOVÁNÍ BANKOVEK
# ---------------------------------------------------------------
# Každý kanál (1–16) může představovat jinou hodnotu podle měnového datasetu
# nahraného v zařízení (např. CZK, PYG, EUR...).
#
# V Python kódu je mapování uvedeno ručně, např.:
# banknotes = {1: 100, 2: 200, 3: 500, 4: 1000, 5: 2000, 6: 5000}
#
# Toto mapování lze změnit podle aktuálně nahraného datasetu NV10.
#
# ---------------------------------------------------------------
# ⚠️ DŮLEŽITÉ POZNÁMKY
# ---------------------------------------------------------------
# • SIO režim (300 Bd, 8N2) je jiný než SSP, ccTalk nebo MDB.
#   Kódy z těchto protokolů nelze zaměňovat!
# • NV10 má vlastní logiku – při zapnutí musí být vždy aktivován (184)
#   a zapnut escrow režim (170), jinak nereaguje na bankovky.
# • Pokud zařízení posílá opakovaně stejné kódy, znamená to buď
#   neukončenou úschovu, nebo chybnou sekvenci příkazů.
# • Při testování vždy vypínej debug mód Flasku (jinak se port COM může zamknout).
#
# ---------------------------------------------------------------
# 🧠 STRUČNÉ SHRNUTÍ
# ---------------------------------------------------------------
# 1. Skript inicializuje NV10 a zapne escrow režim.
# 2. Po vložení bankovky zařízení pošle kód kanálu (např. 1 = 100 Kč).
# 3. Skript rozhodne, zda bankovku přijme (172) nebo odmítne (173).
# 4. Po přijetí je částka převedena na čas (TIME_PER_KC).
# 5. Kredit se uloží do JSON souboru, který čte webové rozhraní.
#
# Díky tomuto postupu je možné bezpečně používat bankovkový validátor
# pro herní automaty, dary, nebo interaktivní projekty bez rizika chybného účtování.


# https://www.coinoperatorshop.com/de/ obchood kde koupi nV10
# https://www.coinoperatorshop.com/de/geldverarbeitung/Banknoten-pruefen-zaehlen-ausgeben/Banknotenleser/Leser/Banknotenpruefer-NV10-USB.html
# https://www.coinoperatorshop.com/de/geldverarbeitung/Banknoten-pruefen-zaehlen-ausgeben/Banknotenleser/Interface---Kabel-1650/IF17-Interface-Converter-IF-17.html



###############################
# Nastavení detekce portu
###############################
USE_EXACT_VID_PID = True  # Pokud je True, hledá se podle vendor/product ID
EXPECTED_VID = 0x0403  # FTDI
EXPECTED_PID = 0x6001  # FT232R

###############################
# 🌍 Flask API Setup
###############################
app = Flask(__name__)
CORS(app)  # Povolit přístup ze všech webových stránek
credit_file = "credit.json"
ser = None
escrow_value = 0

# 🔥 Proměnné pro správu kreditu
credit = 0
total_inserted = 0

###############################
# ⏳ Nastavení času za bankovku
###############################
TIME_PER_KC = 12  # 1 Kč = 12 sekund hraní

###############################
# 💾 Funkce pro ukládání a načítání kreditu
###############################
def save_credit():
    with open(credit_file, "w") as f:
        json.dump({"credit": credit, "total_inserted": total_inserted}, f)

def load_credit():
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
# 🌍 Flask API
###############################
@app.route("/api/credit")
def get_credit():
    return jsonify({"credit": credit, "total_inserted": total_inserted})

@app.route("/api/reset_credit", methods=["POST"])
def reset_credit_api():
    reset_credit()
    return jsonify({"message": "Kredit a celková vložená částka byly resetovány."})

###############################
# 💰 Správa kreditu
###############################
def bill(value):
    global credit, total_inserted
    time_added = value * TIME_PER_KC
    credit += time_added
    total_inserted += value
    minutes = time_added // 60
    print(f"💰 Kredit přidán: {value} Kč -> +{time_added} s ({minutes} min). Celkem: {credit} s, Celkem vloženo: {total_inserted} Kč")
    save_credit()

def start_credit_countdown():
    global credit
    timeout_printed = False
    while True:
        if credit > 0:
            credit -= 1
            timeout_printed = False
            print(f"⏳ Zbývá: {credit} s")
            save_credit()
        else:
            if not timeout_printed:
                print("❌ Čas vypršel, čekám na další vklad...")
                timeout_printed = True
        time.sleep(1)

###############################
# 🔍 Vyhledání portu NV10
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
# 🔄 Smyčka čtení NV10
###############################
def loop_nv10():
    global credit, escrow_value
    while True:
        if ser.in_waiting > 0:
            byte_in = ser.read(1)[0]
            print(f"📥 Přijato od zařízení: {byte_in}")

            # Mapování kanálů (bankovek) – záleží na kalibraci akceptoru
            banknotes = {
                1: 100,
                2: 200,
                3: 500,
                4: 1000,
                5: 2000,
                6: 5000
            }

            # Přijatá bankovka na kanálu (1–16)
            if byte_in in banknotes:
                escrow_value = banknotes[byte_in]
                print(f"💵 Bankovka {escrow_value} Kč v escrow. Odesílám ACCEPT (172).")
                ser.write(bytes([172]))  # příkaz k přijetí bankovky
                print("📤 Odesláno: ACCEPT (172)")

            # Stavové a chybové kódy dle oficiální SIO tabulky
            elif byte_in == 5:
                print("📢 Vrácení bankovky (kód 5).")
            elif byte_in == 20:
                print("📢 Neznámá bankovka (kód 20).")
            elif byte_in == 30:
                print("📢 Mechanismus se pohybuje pomalu (kód 30).")
            elif byte_in == 40:
                print("📢 Pokus o přetažení bankovky (stringing attempt, kód 40).")
            elif byte_in == 50:
                print("📢 Bankovka z kanálu 5 odmítnuta (možný podvod, kód 50).")
            elif byte_in == 60:
                print("📢 Zásobník plný nebo zaseknutý (kód 60).")
            elif byte_in == 70:
                print("❌ Přerušena úschova, bankovka se vrací (kód 70).")
                escrow_value = 0
            elif byte_in == 80:
                print("📢 Bankovka byla možná odebrána kvůli uvolnění zaseknutí (kód 80).")
            elif byte_in == 120:
                print("📢 Validator zaneprázdněn (kód 120).")
            elif byte_in == 121:
                print("📢 Validator volný (kód 121).")

            # Potvrzení přijetí / odmítnutí
            elif byte_in == 172:
                if escrow_value > 0:
                    minutes = (escrow_value * TIME_PER_KC) // 60
                    print(f"✅ Přijato {escrow_value} Kč → +{minutes} minut.")
                    bill(escrow_value)
                    escrow_value = 0
            elif byte_in == 173:
                print("❌ Bankovka odmítnuta (kód 173).")
                escrow_value = 0

            # Ovládací a statusové kódy
            elif byte_in == 170:
                print("📢 Aktivován escrow režim (kód 170).")
            elif byte_in == 171:
                print("📢 Escrow režim deaktivován (kód 171).")
            elif byte_in == 182:
                print("📢 Stavové hlášení (kód 182).")
            elif byte_in == 184:
                print("📢 Akceptor povolen (kód 184).")
            elif byte_in == 185:
                print("📢 Akceptor zakázán (kód 185).")
            elif byte_in == 190:
                print("📢 Časový limit escrow deaktivován (kód 190).")
            elif byte_in == 191:
                print("📢 Časový limit escrow aktivován (kód 191).")
            elif byte_in == 192:
                print("📢 Žádost o verzi firmwaru (kód 192).")
            elif byte_in == 193:
                print("📢 Žádost o verzi datasetu (kód 193).")
            elif byte_in == 255:
                print("⚠️ Chyba příkazu (kód 255).")

            # Neznámý kód
            else:
                print(f"❓ Neznámý nebo nepodporovaný kód: {byte_in}")

        time.sleep(0.05)

###############################
# 🔄 Reset kreditu
###############################
def reset_credit():
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

    load_credit()
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
    threading.Thread(target=loop_nv10, daemon=True).start()
    # threading.Thread(target=start_credit_countdown, daemon=True).start()

    ##########################################
    # 🌍 Automatická detekce systému (Windows / Linux)
    ##########################################
    system_type = platform.system()
    print(f"🖥️ Detekován systém: {system_type}")

    if system_type == "Windows":
        # Na Windows vypneme reloader (jinak blokuje COM)
        print("⚙️ Flask spuštěn s use_reloader=False (Windows režim)")
        app.run(debug=True, use_reloader=False, host="0.0.0.0", port=5000)
    else:
        # Na Linuxu může zůstat plný debug režim
        print("⚙️ Flask spuštěn v plném debug režimu (Linux)")
        app.run(debug=True, use_reloader=True, host="0.0.0.0", port=5000)

###############################
# ▶️ Start
###############################
if __name__ == "__main__":
    main()
