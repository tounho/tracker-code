import htmlmin
from jsmin import jsmin

cpp = '''#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
'''

html_code = ''.join(open('src/index.html').readlines())

print('html uncompressed %i bytes' % len(html_code))

html_code = htmlmin.minify(
    html_code,
    remove_empty_space=True,
    remove_all_empty_space=True,
    reduce_empty_attributes=True,
    reduce_boolean_attributes=True,
    remove_comments=True,
    )

html_code = jsmin(html_code)

print('html compressed %i bytes' % len(html_code))

cpp += html_code



cpp += '''
)rawliteral";
'''

open('src/html.cpp', 'w').write(cpp)
print('%i bytes total' % len(cpp))