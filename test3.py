import serial
import serial.tools.list_ports
import time

###############################
# Volitelná přesná nebo obecná detekce
###############################
# Zde můžete nastavit, zda se má hledat pomocí klíčového slova (např. FTDI),
# nebo podle vendor/product ID.
USE_EXACT_VID_PID = True  # Když nastavíte na False, bude se hledat jen podle keywordu.

# Pokud máte zapnuté USE_EXACT_VID_PID, definuje se, jaké VID a PID hledáme
EXPECTED_VID = 0x0403  # FTDI
EXPECTED_PID = 0x6001  # FT232R

###############################
# Globální proměnné (původní)
###############################
credit = 0              # Celkem přijatá částka
escrow_value = 0        # Hodnota bankovky aktuálně v úschově (pokud existuje)
ser = None              # Globální proměnná pro sériový port

###############################
# Funkce pro nalezení správného portu
###############################
def find_nv10_port(keyword="FTDI"):
    """
    Pokusí se najít zařízení NV10 (nebo převodník FTDI) automaticky.
    Pokud je USE_EXACT_VID_PID = True, hledá přesný VID/PID.
    Jinak hledá klíčové slovo v popisu/manufacturer/name.
    """
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        # Převedeme None na prázdný řetězec "" pomocí ternárního výrazu
        desc = p.description if p.description else ""
        manu = p.manufacturer if p.manufacturer else ""
        name_ = p.name if p.name else ""

        if USE_EXACT_VID_PID:
            # Pokud vyžadujeme přesnou kontrolu vendor & product ID
            if (p.vid == EXPECTED_VID) and (p.pid == EXPECTED_PID):
                return p.device
        else:
            # Hledáme klíčové slovo (keyword) v popisu
            if (keyword in desc) or (keyword in manu) or (keyword in name_):
                return p.device  # Např. '/dev/ttyUSB0'
    return None

###############################
# Funkce pro manipulaci s kreditním zůstatkem
###############################
def bill(value):
    """Přičte hodnotu bankovky k celkovému kreditu."""
    global credit
    credit += value
    print(f"Celkem přijato: {credit} Kč")

###############################
# Inicializace NV10 (setup)
###############################
def setup_nv10():
    """Spuštěno na začátku, posílá základní příkazy."""
    global ser
    try:
        # 184 = povolit akceptor
        ser.write(bytes([184]))
        print("Odesláno: Acceptor enabled (184)")

        # 170 = povolit escrow režim
        ser.write(bytes([170]))
        print("Odesláno: Enable serial escrow mode (170)")

        # 191 = ověřit akceptor
        ser.write(bytes([191]))
        print("Odesláno: Verify acceptor (191)")

        # Pauza pro zpracování
        time.sleep(1)
    except Exception as e:
        print("Chyba při odesílání příkazů:", e)

###############################
# Hlavní smyčka (loop)
###############################
def loop_nv10():
    """Čte data z NV10 a reaguje na ně podobně jako Arduino loop()."""
    global credit, escrow_value, ser

    print("Spouštím hlavní smyčku. Ukončete stiskem Ctrl+C.")

    try:
        while True:
            if ser.in_waiting > 0:
                byte_in = ser.read(1)
                if byte_in:
                    decoded_byte = byte_in[0]
                    print(f"Výstup: {decoded_byte}")

                    # Podle hodnoty bajtu rozhodneme, co se děje
                    if decoded_byte == 1:
                        # Rozpoznána bankovka 100 Kč v escrow
                        print("Bankovka 100 Kč v escrow. Odesílám ACCEPT (172).")
                        escrow_value = 100
                        ser.write(bytes([172]))

                    elif decoded_byte == 2:
                        # Rozpoznána bankovka 200 Kč v escrow
                        print("Bankovka 200 Kč v escrow. Odesílám ACCEPT (172).")
                        escrow_value = 200
                        ser.write(bytes([172]))

                    elif decoded_byte == 3:
                        # Rozpoznána bankovka 500 Kč v escrow
                        print("Bankovka 500 Kč v escrow. Odesílám ACCEPT (172).")
                        escrow_value = 500
                        ser.write(bytes([172]))

                    elif decoded_byte == 70:
                        # 70 = přerušena úschova
                        print("Přerušena úschova. Bankovka se vrací.")
                        escrow_value = 0

                    elif decoded_byte == 172:
                        # 172 = potvrzení, že bankovka byla fyzicky přijata
                        if escrow_value > 0:
                            print(f"Zařízení potvrdilo přijetí bankovky v hodnotě {escrow_value} Kč.")
                            bill(escrow_value)
                            escrow_value = 0

                    elif decoded_byte == 173:
                        # 173 = potvrzení, že bankovka byla odmítnuta
                        print("Bankovka byla odmítnuta (173).")
                        escrow_value = 0

                    elif decoded_byte == 184:
                        print("Zařízení: Acceptor enabled (184).")
                    elif decoded_byte == 185:
                        print("Zařízení: Acceptor disabled (185).")
                    elif decoded_byte == 170:
                        print("Zařízení: Escrow mode enabled (170).")
                    elif decoded_byte == 191:
                        print("Zařízení: Verify acceptor (191).")
                    elif decoded_byte == 182:
                        print("Zařízení: Status (182).")

                    # Další možné kódy 120, 121 atd.
                    elif decoded_byte == 120:
                        print("Zařízení poslalo 120 (neznámý význam, lze si dodefinovat).")
                    elif decoded_byte == 121:
                        print("Zařízení poslalo 121 (neznámý význam, lze si dodefinovat).")

                    else:
                        print("Neznámá odpověď:", decoded_byte)

            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nProgram ukončen uživatelem.")
    except Exception as e:
        print("Chyba v hlavní smyčce:", e)
    finally:
        ser.close()
        print("Sériový port uzavřen.")

###############################
# Spuštění programu
###############################
def main():
    global ser, USE_EXACT_VID_PID

    # 1) Zkusíme najít FTDI převodník automaticky
    detected_port = find_nv10_port()
    if detected_port:
        print(f"Nalezen port: {detected_port}")
        port_to_use = detected_port
    else:
        print("Auto-detekce selhala, zkouším fallback '/dev/ttyUSB0'.")
        port_to_use = '/dev/ttyUSB0'

    try:
        ser = serial.Serial(
            port=port_to_use,
            baudrate=300,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_TWO,
            timeout=1
        )
    except Exception as e:
        print("Chyba při otevírání sériového portu:", e)
        return

    # Nastavíme NV10
    setup_nv10()

    # Spustíme nekonečnou smyčku pro čtení dat
    loop_nv10()

if __name__ == "__main__":
    main()
