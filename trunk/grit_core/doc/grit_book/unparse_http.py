#!/usr/bin/python

import lxml.etree as ET

def UnparseHtmlInlines(ns, inner=False):
    s = ""
    for n in ns:
        if isinstance(n, str) or isinstance(n, unicode):
            s += n
        elif n.kind == 'Definition':
            s += '<span class="def">'
            s += UnparseHtmlInlines(n.data, True)
            s += '</span>'
        elif n.kind == 'Web':
            s += '<a class="web" href="' + n.url + '">'
            s += UnparseHtmlInlines(n.data, True)
            s += '</a>'
        elif n.kind == 'Issue':
            url = 'http://code.google.com/p/grit/issues/detail?id=%d' % n.id
            s += '<a class="issue" href="%s">issue %d</a>' % (url, n.id)
        else:
            print "ERROR: unknown node kind: " + n.kind
    return s

def UnparseHtmlBlocks(ns, path, section_counter = 0):
    s = ""
    for n in ns:
        if n.kind == 'Image':
            s += '''<div class="image">
<div class="image-title">''' + n.title + '''</div>
<a href="''' + n.src + '''"><img class="thumbnail" src="''' + n.thumb_src + '''" /></a>
<div class="image-caption">''' + n.caption + '''</div>
</div>\n'''
        elif n.kind == 'UnorderedList':
            s += '<ul>\n\n'
            for item in n.data:
                s += '<li>\n\n' + UnparseHtmlBlocks(item, path) + '</li>\n\n'
            s += '</ul>\n'
        elif n.kind == 'Section':
            section_counter += 1
            path_string = '.'.join([str(e) for e in path + [section_counter]])
            h = len(path) + 1
            s += '<h%d id="%s">%s&nbsp;&nbsp;%s</h%d>\n\n' % (h, n.id, path_string, n.title, h)
            s += UnparseHtmlBlocks(n.data, path+[section_counter])
        elif n.kind == 'Preformatted':
            s += '<pre>%s</pre>\n' % n.data
        elif n.kind == 'Lua':
            s += '<pre>%s</pre>\n' % n.data
        elif n.kind == 'Paragraph':
            s += '<p>'
            s += UnparseHtmlInlines(n.data)
            s += '</p>\n'
        else:
            print "ERROR: unknown node kind: " + n.kind
        s += '\n'
    return s

