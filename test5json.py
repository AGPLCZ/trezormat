import serial
import serial.tools.list_ports
import json
import time

###############################
# Vysvětlení
###############################
# Tento skript pracuje s validátorem NV10 v režimu escrow (úschova). V praxi to znamená:
# 1. Zařízení detekuje vloženou bankovku (kód 1, 2, 3, atd.).
# 2. Drží ji v úschově a čeká na příkaz přijetí (172) nebo odmítnutí (173).
# 3. Pokud ji přijmeme, připíše se její hodnota do kreditního souboru.
#
# Skript automaticky vyhledá sériový port podle VID/PID nebo klíčového slova.
# Můžeme také nastavit maximální akceptovanou částku a filtrovat přijímané nominály.

###############################
# Nastavení
###############################

USE_EXACT_VID_PID = True
EXPECTED_VID = 0x0403
EXPECTED_PID = 0x6001
PORT_KEYWORD = "FTDI"
MAX_TOTAL = 200  # Maximální částka v MIL PYG

credit_file = "credit.json"
escrow_value = 0
ser = None

# Které bankovky přijímat (True = povolit, False = zakázat)
enabled_banknotes = {
    2: True,   # 2 MIL
    5: True,   # 5 MIL
    10: True,  # 10 MIL
    20: True,  # 20 MIL
    50: True,  # 50 MIL
    100: True # 100 MIL 
}

# Mapování byte kódů NV10 na hodnoty bankovek v MIL PYG
banknotes = {
    1: 2,
    2: 5,
    3: 10,
    4: 20,
    5: 50,
    6: 100
}


def load_credit():
    try:
        with open(credit_file, "r") as f:
            data = json.load(f)
            return data.get("total_inserted", 0)
    except FileNotFoundError:
        return 0


def save_credit(amount):
    with open(credit_file, "w") as f:
        json.dump({"total_inserted": amount}, f)


def find_nv10_port(keyword="FTDI"):
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = p.description if p.description else ""
        manu = p.manufacturer if p.manufacturer else ""
        name_ = p.name if p.name else ""

        if USE_EXACT_VID_PID:
            if (p.vid == EXPECTED_VID) and (p.pid == EXPECTED_PID):
                return p.device
        else:
            if (keyword in desc) or (keyword in manu) or (keyword in name_):
                return p.device
    return None


def setup_nv10():
    global ser
    try:
        ser.write(bytes([184]))  # Enable acceptor
        print("📤 Odesláno: Acceptor enabled (184)")
        time.sleep(0.1)
        ser.write(bytes([170]))  # Enable escrow
        print("📤 Odesláno: Escrow mode enabled (170)")
        time.sleep(0.1)
        ser.write(bytes([191]))  # Verify acceptor
        print("📤 Odesláno: Verify acceptor (191)")
        print("✅ NV10 připraven.")
    except Exception as e:
        print("❌ Chyba při inicializaci:", e)


def main_loop():
    global escrow_value
    total_inserted = load_credit()
    print(f"💾 Načtený kredit: {total_inserted} MIL PYG")

    while True:
        if ser.in_waiting > 0:
            byte_in = ser.read(1)[0]
            print(f"📥 Přijato od zařízení: {byte_in}")

            if byte_in in banknotes:
                escrow_value = banknotes[byte_in]
                if not enabled_banknotes.get(escrow_value, False):
                    print(f"⛔ Bankovka {escrow_value} MIL je zakázána. Odesílám REJECT (173).")
                    ser.write(bytes([173]))
                elif total_inserted + escrow_value > MAX_TOTAL:
                    print(f"⛔ Překročeno maximum {MAX_TOTAL} MIL. Odesílám REJECT (173).")
                    ser.write(bytes([173]))
                else:
                    print(f"💵 Bankovka {escrow_value} MIL v escrow. Odesílám ACCEPT (172).")
                    ser.write(bytes([172]))

            elif byte_in == 70:
                print("❌ Přerušena úschova. Bankovka se vrací.")
                escrow_value = 0

            elif byte_in == 172:
                if escrow_value > 0:
                    total_inserted += escrow_value
                    print(f"✅ Přijato: {escrow_value} MIL. Celkem: {total_inserted} MIL PYG.")
                    save_credit(total_inserted)
                    escrow_value = 0

            elif byte_in == 173:
                print("❌ Bankovka odmítnuta (173).")
                escrow_value = 0

            elif byte_in == 184:
                print("📢 Acceptor enabled.")
            elif byte_in == 185:
                print("📢 Acceptor disabled.")
            elif byte_in == 170:
                print("📢 Escrow mode enabled.")
            elif byte_in == 191:
                print("📢 Verify acceptor.")
            elif byte_in == 182:
                print("📢 Status.")
            elif byte_in == 120:
                print("📢 Neznámá bankovka (120).")
            elif byte_in == 121:
                print("📢 Kód 121 Vrácena neznámá bankovka.")
            elif byte_in == 20:
                print("📢 Kód 20 Vracím neznámou bankovku.")
            elif byte_in == 5:
                print("📢 Vracím bankovku (5).")
            else:
                print(f"❓ Neznámý kód: {byte_in}")

        time.sleep(0.05)


def main():
    global ser
    port = find_nv10_port(PORT_KEYWORD)
    if not port:
        print("❌ Port NV10 nebyl nalezen.")
        return

    try:
        ser = serial.Serial(port, baudrate=300, bytesize=serial.EIGHTBITS,
                            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_TWO, timeout=1)
        print(f"🔌 Připojeno k {port}")
    except Exception as e:
        print(f"❌ Nelze otevřít port: {e}")
        return

    setup_nv10()
    main_loop()


main()
