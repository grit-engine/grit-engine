#!/usr/bin/python

import lxml.etree as ET

def UnparseHtmlInlines(ns, inner=False):
    s = ""
    for n in ns:
        if isinstance(n, str):
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

def UnparseHtmlBlocks(ns, d=1):
    s = ""
    section_counter = 0
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
                s += '<li>\n\n' + UnparseHtmlBlocks(item) + '</li>\n\n'
            s += '</ul>\n'
        elif n.kind == 'Section':
            s += '<h%d>%s</h%d>\n\n' % (d, n.title, d)
            s += UnparseHtmlBlocks(n.data, d+1)
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

