// After throwing a preset amount, a message is displayed and, for example, a relay can be switched (or a web address can be retrieved - GET)
// This code works by activating an interrupt (function coinInsertInterrupt) upon coin detection, incrementing the total number of detected coins. In the main loop (loop), it checks if enough time has passed since the last detection and if the current balance has reached the required amount. If so, it sends a message and resets the current balance and the total number of detected coins.

const int coinDetectionPin = 2; // Pin connected to the coin detector
const int requiredAmount = 75; // Amount that needs to be reached to release product
volatile long totalInterruptsCount = 0; // Total number of detected coins
unsigned long currentBalance = 0; // Current accumulated value of coins
unsigned long lastInterruptTime = 0; // Time of the last detected coin

void setup() {
  Serial.begin(9600); // Initialize serial port with 9600 baud rate
  attachInterrupt(digitalPinToInterrupt(coinDetectionPin), coinInsertInterrupt, RISING); // Set up interrupt for coin detection
}

void loop() {
  // Check if sufficient time has passed since the last interrupt and if any coin was detected
  if (millis() - lastInterruptTime > 1 && totalInterruptsCount > 0) {
    currentBalance += totalInterruptsCount; // Add the number of detected coins to the current balance
    if (currentBalance >= requiredAmount) { // If current balance is equal to or greater than the required amount
      Serial.println("balanceReached"); // Send message indicating that the required amount has been reached
      currentBalance = 0; // Reset the current balance
    }
    Serial.println("Current Balance: " + String(currentBalance) + " Kc"); // Print the current balance
    totalInterruptsCount = 0; // Reset the count of detected coins
  }
}

void coinInsertInterrupt() {
  totalInterruptsCount++; // Record new detected coin
  lastInterruptTime = millis(); // Update time of the last detection
}
