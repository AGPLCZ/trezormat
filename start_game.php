<?php
$script = __DIR__ . '/start_game.sh';
exec("export DISPLAY=:0; $script > /dev/null 2>&1 &");

header('Location: index.php');
exit;
?>
