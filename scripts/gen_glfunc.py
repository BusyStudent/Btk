#!/usr/bin/env python3
if __name__ == '__main__':
    s = open('/usr/include/GLES3/gl3.h').read()
    for i in s.splitlines():
        pos = i.find('GL_APIENTRY gl')
        l   = len('GL_APIENTRY gl')
        if pos != -1:
            fn = i[pos + l:i.find(' ',pos + l)]
            fn = 'gl' + fn
            print('BTK_GL_PROCESS(PFN%sPROC,%s) \\' % (fn.upper(),fn))