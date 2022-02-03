from decimal import DivisionByZero

try:
    1/0
except DivisionByZero:
    raise DivisionByZero