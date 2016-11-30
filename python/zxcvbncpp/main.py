import math
import sys

from zxcvbncpp._zxcvbncpp import lib, ffi

if sys.version_info[0] >= 3:
    str_type = str
else:
    str_type = unicode

def _maybe_encode(password):
    if isinstance(password, str_type):
        return password.encode('utf8')
    return password

def round_to_x_digits(number, digits):
    """
    Returns 'number' rounded to 'digits' digits.
    """
    return round(number * math.pow(10, digits)) / math.pow(10, digits)

def password_strength(password, user_inputs=()):
    user_inputs = list(map(lambda x: ffi.new("char[]", _maybe_encode(x)), user_inputs))
    user_inputs.append(ffi.NULL)
    guesses = ffi.new("zxcvbn_guesses_t *")
    err = lib.zxcvbn_password_strength(_maybe_encode(password), user_inputs,
                                       guesses, ffi.NULL)
    if err:
        raise Exception("Error!")
    return dict(
        password=password,
        guesses=guesses[0],
        entropy=round_to_x_digits(math.log(guesses[0], 2), 3),
    )

if __name__ == "__main__":
    result = password_strength("correcthorsebatterystaple")
    print(result)
