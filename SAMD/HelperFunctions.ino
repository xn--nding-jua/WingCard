String split(String s, char parser, int index) {
  String rs="";
  int parserCnt=0;
  int rFromIndex=0, rToIndex=-1;
  while (index >= parserCnt) {
    rFromIndex = rToIndex+1;
    rToIndex = s.indexOf(parser,rFromIndex);
    if (index == parserCnt) {
      if (rToIndex == 0 || rToIndex == -1) return "";
      return s.substring(rFromIndex,rToIndex);
    } else parserCnt++;
  }
  return rs;
}

String intToHex(uint32_t val, uint8_t outputLength) {
  String hexString;
  for (int8_t shift = outputLength * sizeof(val) - 4; shift >= 0; shift -= 4) {
    uint8_t hexDigit = (val >> shift) & 0xF;
    hexString += String(hexDigit, HEX);
  }
  return hexString;
}

uint32_t hexToInt(String hexString){
  char c[hexString.length() + 1];
  hexString.toCharArray(c, hexString.length() + 1);
  return strtol(c, NULL, 16); 
}

void charToString(char *data, String &s)
{
  uint8_t ptr = 0;
  s = ""; // init string
  while (data[ptr]) {
    s.concat(data[ptr++]);
  }
}
