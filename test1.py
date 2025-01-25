import serial
import time

###############################
# Vysvětlení
###############################
# Tento skript pracuje s validátorem NV10 v režimu escrow (úschova). V praxi to znamená:
# 1. Zařízení detekuje vloženou bankovku (kód 1, 2, 3 podle nominální hodnoty).
# 2. Drží ji v "úschově" (escrow) a čeká, zda ji přijmeme (172) nebo odmítneme (173).
# 3. Teprve poté, co zařízení zašle kód o konečném stavu (např. 172 pro přijetí), přičteme hodnotu.
#
# Tímto způsobem by se mělo zabránit vícenásobnému započítání jedné bankovky.
# Pokud přesto NV10 posílá opakované kódy bankovky, můžeme si je ošetřit další logikou.
# Zde je základní funkčnost co nejblíže Arduino kódu.

# 'E': Aktivuje akceptor (184)
# 'D': Deaktivuje akceptor (185)
# 'M': Zapne režim úschovy (170)
# 'S': Požádá o status (182)
# 'Y': Přijme bankovku v úschově (172)
# 'N': Odmítne bankovku v úschově (173)

###############################
# Globální proměnné
###############################
credit = 0              # Celkem přijatá částka
escrow_value = 0        # Hodnota bankovky aktuálně v úschově (pokud existuje)

###############################
# Inicializace sériového portu
###############################
try:
    ser = serial.Serial(
        port='/dev/ttyUSB0',     # Přizpůsobte podle potřeby
        baudrate=300,            # NV10 často používá 300 baud
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_TWO,
        timeout=1                # 1 sekunda timeoutu pro čtení
    )
except Exception as e:
    print("Chyba při otevírání sériového portu:", e)
    exit(1)

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
    """Čte data z NV10 a reaguje."""
    global credit, escrow_value

    print("Spouštím hlavní smyčku.")

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
                        # 172 = potvrzení přijetí bankovky
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
    setup_nv10()
    loop_nv10()

if __name__ == "__main__":
    main()
