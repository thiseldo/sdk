// Copyright © 2016 The Things Network
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.

#include "Arduino.h"
#include "TheThingsUno.h"

void TheThingsUno::init(Stream& modemStream, Stream& debugStream) {
  this->modemStream = &modemStream;
  this->debugStream = &debugStream;
}

String TheThingsUno::readLine(int waitTime) {
  unsigned long start = millis();
  while (millis() < start + waitTime) {
    String line = modemStream->readStringUntil('\n');
    if (line.length() > 0) {
      return line.substring(0, line.length() - 1);
    }
  }
  return "";
}

bool TheThingsUno::waitForOK(int waitTime, String okMessage) {
  String line = readLine(waitTime);
  if (line == "") {
    debugPrintLn("Wait for OK time-out expired");
    return false;
  }

  if (line != okMessage) {
    debugPrintLn("Response is not OK");
    return false;
  }

  return true;
}

String TheThingsUno::readValue(String cmd) {
  modemStream->println(cmd);

  return readLine();
}

bool TheThingsUno::sendCommand(String cmd, int waitTime) {
  debugPrintLn("Sending: " + cmd);

  modemStream->println(cmd);

  return waitForOK(waitTime);
}

bool TheThingsUno::sendCommand(String cmd, String value, int waitTime) {
  int l = value.length();
  byte buf[l];
  value.getBytes(buf, l);

  return sendCommand(cmd, buf, l, waitTime);
}

char btohexa_high(unsigned char b) {
    b >>= 4;
    return (b > 0x9u) ? b + 'A' - 10 : b + '0';
}

char btohexa_low(unsigned char b) {
    b &= 0x0F;
    return (b > 0x9u) ? b + 'A' - 10 : b + '0';
}

bool TheThingsUno::sendCommand(String cmd, const byte* buf, int length, int waitTime) {
  debugPrintLn("Sending: " + cmd + " with " + String(length) + " bytes");

  modemStream->print(cmd + " ");

  for (int i = 0; i < length; i++) {
    modemStream->print(btohexa_high(buf[i]));
    modemStream->print(btohexa_low(buf[i]));
    debugStream->print(btohexa_high(buf[i]));
    debugStream->print(btohexa_low(buf[i]));
  }
  modemStream->println();
  debugStream->println();

  return waitForOK(waitTime);
}

void TheThingsUno::reset(bool adr, int fsb) {
  modemStream->println("sys reset");
  String version = readLine(3000);
  if (version == "") {
    debugPrintLn("Invalid version");
    return;
  }

  model = version.substring(0, version.indexOf(' '));
  debugPrintLn("Version is " + version + ", model is " + model);

  sendCommand("mac set adr " + String(adr ? "on" : "off"));

  if (model == "RN2483")
    sendCommand("mac set pwridx " + String(PWRIDX_868));
  else if (model == "RN2903") {
    sendCommand("mac set pwridx " + String(PWRIDX_915));
    enableFsbChannels(fsb);
  }
}

bool TheThingsUno::enableFsbChannels(int fsb) {
  if (fsb == 0)
    return true;

  int chLow, chHigh;
  switch (fsb) {
    case 7:
      chLow = 48;
      chHigh = 55;
      break;
    default:
      debugPrintLn("Invalid FSB");
      return false;
  }

  for (int i = 0; i < 72; i++) {
    String command = "mac set ch status " + String(i);
    if (i == 70 || chLow <= i && i <= chHigh)
      sendCommand("mac set ch status " + String(i) + " on");
    else
      sendCommand("mac set ch status " + String(i) + " off");
  }
  return true;
}

bool TheThingsUno::personalize(const byte devAddr[4], const byte nwkSKey[16], const byte appSKey[16]) {
  sendCommand("mac set devaddr", devAddr, 4);
  sendCommand("mac set nwkskey", nwkSKey, 16);
  sendCommand("mac set appskey", appSKey, 16);
  sendCommand("mac join abp");

  if (readLine() != "accepted") {
    debugPrintLn("Personalize not accepted");
    return false;
  }

  debugPrintLn("Personalize accepted. Status: " + readValue("mac get status"));
  return true;
}

bool TheThingsUno::join(const byte appEui[8], const byte appKey[16]) {
  sendCommand("mac set appeui", appEui, 8);
  sendCommand("mac set appkey", appKey, 16);
  sendCommand("mac join otaa", 10000);

  if (readLine() != "accepted") {
    debugPrintLn("Join not accepted");
    return false;
  }

  debugPrintLn("Join accepted. Status: " + readValue("mac get status"));
  return true;
}

void TheThingsUno::sendBytes(const byte* buffer, int length, int port, bool confirm) {
  if (!sendCommand("mac tx " + String(confirm ? "cnf" : "uncnf") + " " + String(port), buffer, length)) {
    debugPrintLn("Send command failed");
    return;
  }

  if (waitForOK(10000, "mac_tx_ok"))
    debugPrintLn("Successful transmission");
}

void TheThingsUno::sendString(String message, int port, bool confirm) {
  int l = message.length();
  byte buf[l + 1];
  message.getBytes(buf, l + 1);

  return sendBytes(buf, l, port, confirm);
}

void TheThingsUno::showStatus() {
  debugPrintLn("EUI: " + readValue("sys get hweui"));
  debugPrintLn("Battery: " + readValue("sys get vdd"));
  debugPrintLn("AppEUI: " + readValue("mac get appeui"));
  debugPrintLn("DevEUI: " + readValue("mac get deveui"));
  debugPrintLn("DevAddr: " + readValue("mac get devaddr"));

  if (this->model == "RN2483") {
    // TODO: Test this
    debugPrintLn("Channel: " + readValue("mac get ch"));
    debugPrintLn("Band: " + readValue("mac get band"));
  }

  debugPrintLn("Data Rate: " + readValue("mac get dr"));
  debugPrintLn("RX Delay 1: " + readValue("mac get rxdelay1"));
  debugPrintLn("RX Delay 2: " + readValue("mac get rxdelay2"));
}
