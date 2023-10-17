
<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

$url = 'https://wallet.paralelnipolis.cz/withdraw/api/v1/links';
$apiKey = 'e***********************6';

$headers = [
  "X-Api-Key: $apiKey", 
  "Content-Type: application/json"
];

$body = [
  'title' => 'Můj automatický odkaz',
  'min_withdrawable' => 20,
  'max_withdrawable' => 20,
  'uses' => 1,
  'wait_time' => 60,
  'is_unique' => true,
  'webhook_url' => 'https://dobrodruzi.cz/withdraw/index.php',
  'webhook_headers' => 'Content-Type: application/json',
  'webhook_body' => '{\"amount\": 100}',
  'custom_url' => 'https://dobrodruzi.cz/withdraw/poslano.php',
];

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($body));
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

if ($httpCode == 201) {
    $responseDecoded = json_decode($response, true);
    if (isset($responseDecoded['lnurl'])) {
        //echo "Odkaz byl vytvořen: " . $responseDecoded['lnurl'];
        echo "" . $responseDecoded['lnurl'];
    } else {
        echo "Neočekávaná odpověď od serveru.";
    }
} elseif ($httpCode == 422) { // Validation Error
    $responseDecoded = json_decode($response, true);
    echo "Validační chyba:<br>";
    foreach ($responseDecoded['detail'] as $errorDetail) {
        echo "Položka: " . implode('.', $errorDetail['loc']) . " - " . $errorDetail['msg'] . "<br>";
    }
} else {
    echo "HTTP chybový kód: $httpCode<br>";
    $curlError = curl_error($ch);
    if ($curlError) {
        echo "Chyba: " . $curlError . "<br>";
    }
    echo "Odpověď z API: $response";
}

curl_close($ch);




/*


curl -X 'POST' \
  'https://lnbits.cz/withdraw/api/v1/links?api-key=a2e4a5fd6f804685bd6de120a2d5accd' \
  -H 'accept: application/json' \
  -H 'X-API-KEY: a2e4a5fd6f804685bd6de120a2d5accd' \
  -H 'Content-Type: application/json' \
  -d '{
  "title": "string",
  "min_withdrawable": 1,
  "max_withdrawable": 1,
  "uses": 1,
  "wait_time": 1,
  "is_unique": true,
  "webhook_url": "string",
  "webhook_headers": "string",
  "webhook_body": "string",
  "custom_url": "string"
}'

curl -X 'POST' \
  'https://legend.lnbits.com/withdraw/api/v1/links?api-key=e41fae56dffa4921be0caea2bcf45edf' \
  -H 'accept: application/json' \
  -H 'X-API-KEY: e41fae56dffa4921be0caea2bcf45edf' \
  -H 'Content-Type: application/json' \
  -d '{
  "title": "string",
  "min_withdrawable": 1,
  "max_withdrawable": 1,
  "uses": 1,
  "wait_time": 1,
  "is_unique": true,
  "webhook_url": "string",
  "webhook_headers": "string",
  "webhook_body": "string",
  "custom_url": "string"
}'
*/


?>

