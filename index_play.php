<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Automatické přehrávání videa</title>
    <link href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.1/css/all.min.css">
    <style>
        body,
        html {
            margin: 0;
            padding: 0;
            overflow: hidden;
            background: black;
        }

        video {
            position: absolute;
            top: 50%;
            left: 50%;
            min-width: 70%;
            min-height: 70%;
            width: auto;
            height: auto;
            transform: translate(-50%, -50%);
            z-index: -1;
        }

        .play-button {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            z-index: 1;
        }
    </style>
</head>

<body>

    <video id="videoPlayer" autoplay loop muted>
        <source src="okruh0.mp4" type="video/mp4"> Game
    </video>

    <div class="play-button">
        <a href="print.php" class="btn btn-success">Jugar - más información <i class="fas fa-gamepad"></i> (Y)</a>
    </div>
    <script>
    const video = document.getElementById('videoPlayer');

    video.addEventListener('loadedmetadata', function() {
        // získání délky videa
        const duration = video.duration;

        // vytvoření náhodného startovního času mezi 0 a délkou videa mínus jedna minuta (60 vteřin)
        const randomStartTime = Math.random() * (duration - 60);

        // nastavení aktuálního času na náhodný startovní čas
        video.currentTime = randomStartTime;
    });
</script>
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

    
    <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.3/dist/umd/popper.min.js"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>
</body>

</html>





