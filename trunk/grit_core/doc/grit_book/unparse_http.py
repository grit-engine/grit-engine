#!/usr/bin/python

import codecs

import lxml.etree as ET

def GeneratePage(title, content, book):
    s = '''<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
    <title>Grit Manual - ''' + title + '''</title>
    <meta http-equiv="Content-type" content="text/html;charset=UTF-8" />
    <link rel="stylesheet" type="text/css" href="grit_book.css" />
    <link rel="icon" type="image/png" href="logo_tiny.png" />
</head>

<body>

<div class="page">

<div class="header">

<div class="logo" > </div>

<div class="navbar">'''
    if book:
        s += '\n<div class="navitem"><a class="navitem" href="index.html">Contents</a></div>'
        for chapter_index, chapter in enumerate(book):
            filename = chapter.id + '.html'
            s += '\n<div class="navitem"><a class="navitem" href="%s">%d. %s</a></div>' % (filename, chapter_index + 1, chapter.title)
    else:
        s += '\n<div class="navitem"><a class="navitem" href="complete.html">Complete Manual</a></div>'
    s += '''
</div>


</div>

<div class="content">

''' + content + '''

</div>

</div>

</body>

</html>'''
    return s


def UnparseHtmlInlines(parent, inner=False):
    s = ""
    for n in parent:
        if isinstance(n, basestring):
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


def UnparseHtmlBlocks(book, parent, path, section_counter, split_below, never_split):
    s = ""
    for n in parent:
        if n.kind == 'Image':
            s += '''<div class="image">
<div class="image-title">''' + n.title + '''</div>
<a class="image-link" href="''' + n.src + '''"><img class="thumbnail" src="''' + n.thumb_src + '''" /></a>
<div class="image-caption">''' + n.caption + '''</div>
</div>\n'''
        elif n.kind == 'UnorderedList':
            s += '<ul>\n\n'
            for item in n.data:
                s += '<li>\n\n'
                s += UnparseHtmlBlocks(book, item, path, section_counter, split_below, never_split)
                s += '</li>\n\n'
            s += '</ul>\n'
        elif n.kind == 'Section':
            section_counter += 1
            new_path = path + [section_counter] 
            path_string = '.'.join([str(e) for e in new_path])
            h = len(new_path)
            inner_html = '<h%d id="%s">%s.&nbsp;&nbsp;%s</h%d>\n\n' % (h, n.id, path_string, n.title, h)
            inner_html += UnparseHtmlBlocks(book, n, new_path, 0, n.split, never_split)
            if split_below and not never_split:
                filename = n.id + '.html'
                print 'Writing ' + filename
                f = codecs.open(filename, 'w', 'utf-8')
                f.write(GeneratePage(n.title, inner_html, book))
                f.close()
            else:
                s += inner_html
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
    if split_below and not never_split:
        s += BuildHtmlIndex(parent, path)
    return s


def BuildHtmlIndex(parent, path, filename=None):
    index = ''
    section_index = 0
    for n in parent:
        if n.kind == 'Section':
            section_index += 1
            new_path = path + [section_index]
            path_string = '.'.join([str(e) for e in new_path])
            indent = '&nbsp;' * (len(path) * 8)
            new_filename = n.id + '.html' if parent.split else filename
            if parent.split:
                index += '<p class="index">%s%s. <a class="index" href="%s">%s</a></p>\n\n' % (indent, path_string, new_filename, n.title)
            else:
                url = '%s#%s' % (new_filename, n.id)
                index += '<p class="index">%s%s. %s (<a class="index" href="%s">&sect;</a>)</p>\n\n' % (indent, path_string, n.title, url)
            index += BuildHtmlIndex(n, new_path, new_filename)
    return index

