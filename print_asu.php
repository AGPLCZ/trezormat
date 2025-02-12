<?php

// Nastavení: 1 MIL PYG = 120 sekund (tj. 2 minuty)
// Pokud změníte tuto hodnotu, ceny se přepočítají
$time_per_mil = 120; // 120 sekund na 1 MIL PYG

// Pole s nabízenými délkami hraní v minutách
$durations = [10, 20, 30, 40, 50, 60];

// V cyklu projdeme všechny časové intervaly


?>
<!DOCTYPE html>
<html lang="cs">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Zaplaceno</title>
    <link href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="fontawesome/css/all.min.css">
    <style>
        @keyframes changeBackground {
            0% {
                background-image: url('wal1.png');
            }

            25% {
                background-image: url('wal2.webp');
            }

            50% {
                background-image: url('wal3.jpg');
            }

            75% {
                background-image: url('wal1.png');
            }

            100% {
                background-image: url('wal4.jpg');
            }
        }

        body {
            animation: changeBackground 100s infinite;
            background-size: cover;
            overflow-x: hidden;
            overflow-x: hidden;
        }

        .transparent-bg {
            padding: 15px;
            background-color: rgba(255, 255, 255, 0.9);
            border: 1px solid #ccc;
            border-radius: 5px;
        }
    </style>


</head>

<body>

    <div class=""
        style="background-color: rgba(255, 255, 255, 0); max-width: 320px; margin-left:auto; margin-top: 0px; margin-right: 0px">

        <div class="col-md-0 mt-0 px-2 py-2" style="max-width: 320px;">
            <div class="mt-5 px-2 mr-5 py-2 transparent-bg" style="background-color: rgba(255, 255, 255, 0.4);">


                <div class="d-flex justify-content-center transparent-bg"
                    style="background-color: rgba(255, 255, 255, 0.99); margin-top:10px;">
                    <div>
                        <ul class="list-group">
                            <a href="#" class="list-group-item list-group-item-action active">
                                Quien juega no se enoja
                            </a>


                            <?php

                            foreach ($durations as $minutes) {
                                // Výpočet ceny:
                                // Cena (v MIL PYG) = (celkový herní čas v sekundách) / ($time_per_mil)
                                // Tedy: ($minutes * 60) / $time_per_mil.
                                $mil = ($minutes * 60) / $time_per_mil;
                                // Zaokrouhlení na celé číslo (protože při 120 sekundách to dává přesně 0.5násobek)
                                $mil = (int) $mil;

                                // Nastavení třídy a formátování podle daného intervalu:
                                // První položka se přeškrtnutou cenou (badge-secondary)
                                // Druhá položka má speciální třídu (badge-danger)
                                // Ostatní používají badge-primary
                                if ($minutes == 0) {
                                    $badgeClass = "badge-secondary";
                                    $priceDisplay = "<s>{$mil} MIL PYG</s>";
                                }
                                if ($minutes == 0) {
                                    $badgeClass = "badge-danger";
                                    $priceDisplay = "{$mil} MIL PYG";
                                } else {
                                    $badgeClass = "badge-primary";
                                    $priceDisplay = "{$mil} MIL PYG";
                                }

                                // Výpis HTML položky
                                echo '<li class="list-group-item d-flex justify-content-between align-items-center">';
                                echo "{$minutes} minutos";
                                echo '<span class="badge ' . $badgeClass . ' badge-pill">' . $priceDisplay . '</span>';
                                echo '</li>';
                            }


                            ?>

                            <div style="margin-top: 30px; padding-left: 10px; ">
                                Billetes ya insertados : <p><span class="badge badge-danger badge-pill"><span
                                            id="totalInserted">0 MIL
                                            PYG</span></p>

                                Tiempo restante: <p> <span class="badge badge-danger badge-pill"><span id="credit">0 min
                                            0
                                            sec</span></span></p>
                            </div>
                        </ul>
                        <p id="status"></p>

                        <div class="d-flex justify-content-center" style="margin-bottom: 10px">
                            <!-- Přidáno ID pro tlačítko -->
                            <a href="start_game.php" class="btn btn-primary" id="startGameBtn">
                                Iniciar el juego <i class="fas fa-gamepad"></i> (A)
                            </a>
                        </div>

                    </div>


                </div>
            </div>
        </div>

        <div class="d-flex justify-content-end" style="margin-right:30px;">
            <p style="margin-top:10px;"> <a href="index.php" class="btn btn-danger">atrás <i class="fas fa-gamepad"></i>
                    (B)</a></a></p>
        </div>

        <div class="d-flex justify-content-center" style="margin-bottom: 10px">

            <a href="start_game.php" class="btn btn-secondary">No tengo dinero, juego gratis<i
                    class="fas fa-gamepad"></i>
                (Y)</a></a>
        </div>


    </div>
    <div>

        <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js"></script>
        <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.3/dist/umd/popper.min.js"></script>
        <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>

        <script>
            // Funkce pro zpracování vstupu z gamepadu
            function handleGamepadInput() {
                // Získání seznamu všech připojených gamepadů
                const gamepads = navigator.getGamepads();

                // Pokud je nějaký gamepad připojen
                if (gamepads[0]) {
                    const gamepad = gamepads[0];

                    // Pokud je stisknuto tlačítko "A" (index 0)
                    if (gamepad.buttons[0].pressed) {
                        // Simuluje kliknutí na první tlačítko na stránce (pro účely demonstrace)
                        const btnA = document.querySelector('a.btn-primary');
                        if (btnA) btnA.click();
                    }

                    // Pokud je stisknuto tlačítko "B" (index 1)
                    if (gamepad.buttons[1].pressed) {
                        const btnB = document.querySelector('a.btn-danger');
                        if (btnB) btnB.click();
                    }

                    // Pokud je stisknuto tlačítko "X" (index 2)
                    if (gamepad.buttons[2].pressed) {
                        const btnX = document.querySelector('a.btn-secondary');
                        if (btnX) btnX.click();
                    }


                    // Pokud je stisknuto tlačítko "Y" (index 3)
                    if (gamepad.buttons[3].pressed) {
                        const btnY = document.querySelector('a.btn-success');
                        if (btnY) btnY.click();
                    }
                }

                // Opětovné zavolání funkce za 100ms
                setTimeout(handleGamepadInput, 100);
            }

            // Inicializace funkce po načtení stránky
            window.addEventListener('load', handleGamepadInput);
        </script>

    </div>




    <!-- Vložené skripty -->
    <script>
        // Globální proměnná indikující, zda má uživatel kredit
        let userHasCredit = false;

        async function updateCredit() {
            try {
                let response = await fetch("http://localhost:5000/api/credit");
                let text = await response.text();
                console.log("Odpověď API:", text);
                let data = JSON.parse(text);

                // Převod sekund na minuty a sekundy
                let totalSeconds = data.credit;
                let minutes = Math.floor(totalSeconds / 60);
                let seconds = totalSeconds % 60;
                let formattedTime = `${minutes} min ${seconds} sec`;

                document.getElementById("credit").textContent = formattedTime;
                document.getElementById("totalInserted").textContent = data.total_inserted + " MIL PYG";

                // Uživatel má kredit, pokud je počet sekund větší než 0
                userHasCredit = totalSeconds > 0;

                document.getElementById("status").textContent = userHasCredit ? "" : "";
            } catch (error) {
                console.error("Chyba při načítání kreditu:", error);
                document.getElementById("credit").textContent = "Chyba!";
            }
        }
        setInterval(updateCredit, 2000);
        updateCredit();

        // Zajištění, že se tlačítko nespustí, pokud není zaplaceno
        document.addEventListener('DOMContentLoaded', function () {
            const startGameBtn = document.getElementById('startGameBtn');
            startGameBtn.addEventListener('click', function (e) {
                if (!userHasCredit) {
                    e.preventDefault();
                    //alert("Nemáte dostatek kreditu, prosím vložte peníze.");
                }
            });
        });

        // Zpracování vstupu z gamepadu
        async function handleGamepadInput() {
            const gamepads = navigator.getGamepads();
            if (gamepads[0]) {
                const gamepad = gamepads[0];
                // Tlačítko "A" – spustí hru
                if (gamepad.buttons[0].pressed) {
                    const btnA = document.getElementById('startGameBtn');
                    if (btnA) btnA.click();
                }
                // Další tlačítka dle potřeby...
                if (gamepad.buttons[1].pressed) {
                    const btnB = document.querySelector('a.btn-danger');
                    if (btnB) btnB.click();
                }
                if (gamepad.buttons[2].pressed) {
                    const btnX = document.querySelector('a.btn-secondary');
                    if (btnX) btnX.click();
                }
                if (gamepad.buttons[3].pressed) {
                    const btnY = document.querySelector('a.btn-success');
                    if (btnY) btnY.click();
                }
            }
            setTimeout(handleGamepadInput, 100);
        }
        window.addEventListener('load', handleGamepadInput);
    </script>

    <!-- Bootstrap a další skripty -->
    <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.3/dist/umd/popper.min.js"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>
</body>

</html>
