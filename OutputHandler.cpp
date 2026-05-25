#include "OutputHandler.h"

void OutputHandler::setup(Print* printer) {
	_out = printer;
}

void OutputHandler::fprint(const char *format, va_list args) {
	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			printFormat(*format, &args);
		} else {
			_out->print(*format);
		}
	}
}

void OutputHandler::printFormat(const char format, va_list *args) {
	if (format == '\0') return;
	if (format == '%') {
		_out->print(format);
	} else if (format == 's') {
		char *s = va_arg(*args, char *);
		_out->print(s);
	} else if (format == 'd' || format == 'i') {
		_out->print(va_arg(*args, int), DEC);
	} else if (format == 'D' || format == 'F') {
		_out->print(va_arg(*args, double));
	} else if (format == 'x') {
		_out->print(va_arg(*args, int), HEX);
	} else if (format == 'X') {		
		_out->print("0x");
	    uint16_t h = (uint16_t) va_arg( *args, int );
        if (h<0xFFF) _out->print('0');
        if (h<0xFF ) _out->print('0');
        if (h<0xF  ) _out->print('0');
        _out->print(h,HEX);
	} else if (format == 'p') {		
		Printable *obj = (Printable *) va_arg(*args, int);
		_out->print(*obj);
	} else if (format == 'b') {
		_out->print(va_arg(*args, int), BIN);
	} else if (format == 'B') {
		_out->print("0b");
		_out->print(va_arg(*args, int), BIN);
	} else if (format == 'l') {
		_out->print(va_arg(*args, long), DEC);
	} else if (format == 'u') {
		_out->print(va_arg(*args, unsigned long), DEC);
	} else if (format == 'c') {
		_out->print((char) va_arg(*args, int));
	} else if( format == 'C' ) {
		char c = (char) va_arg( *args, int );
		if (c>=0x20 && c<0x7F) {
			_out->print(c);
		} else {
			_out->print("0x");
			if (c<0xF) _out->print('0');
			_out->print(c, HEX);
		}
    } else if(format == 't') {
		if (va_arg(*args, int) == 1) {
			_out->print("T");
		} else {
			_out->print("F");
		}
	} else if (format == 'T') {
		if (va_arg(*args, int) == 1) {
			_out->print(F("true"));
		} else {
			_out->print(F("false"));
		}
	}
}