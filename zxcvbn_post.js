zxcvbn = zxcvbn()["password_strength"];

if (typeof module !== 'undefined') {
  module["exports"] = zxcvbn;
}
