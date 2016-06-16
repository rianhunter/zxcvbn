import sys

from zxcvbn._zxcvbn import lib, ffi

if sys.version_info[0] >= 3:
    str_type = str
else:
    str_type = unicode

def _maybe_encode(password):
    if isinstance(password, str_type):
        return password.encode('utf8')
    return password

def password_strength(password, user_inputs=()):
    user_inputs = list(map(lambda x: ffi.new("char[]", _maybe_encode(x)), user_inputs))
    user_inputs.append(ffi.NULL)
    guesses = ffi.new("zxcvbn_guesses_t *")
    err = lib.zxcvbn_password_strength(_maybe_encode(password), user_inputs,
                                       guesses, ffi.NULL)
    if err:
        raise Exception("Error!")
    return dict(guesses=guesses[0])

if __name__ == "__main__":
    import math
    result = password_strength("correcthorsebatterystaple")
    print(result['guesses'])
    print(math.log10(result['guesses']))
