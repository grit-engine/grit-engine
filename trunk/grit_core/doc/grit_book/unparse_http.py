#!/usr/bin/python

import codecs
import textwrap
from xml.sax.saxutils import escape
import lxml.etree as ET

from translate_xml import *

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
    <div class="logo"> </div>
    <div class="navbar">'''
    if book:
        s += '\n    <div class="navitem"><a class="navitem" href="index.html">Contents</a></div>'
        for chapter_index, chapter in enumerate(book):
            filename = chapter.id + '.html'
            s += '\n        <div class="navitem"><a class="navitem" href="%s">%d. %s</a></div>' % (filename, chapter_index + 1, chapter.title)
    else:
        s += '\n    <div class="navitem"><a class="navitem" href="complete.html">Complete Manual</a></div>'
    s += '''
    </div>
</div>


<div class="content">

''' + content + '''</div> <!-- content -->


</div> <!-- page -->

</body>

</html>
'''
    return s


def UnparseHtmlInlines(parent, inner=False):
    s = ""
    for n in parent:
        if isinstance(n, basestring):
            s += escape(n)
        elif n.kind == 'Definition':
            s += '<span class="def">'
            s += UnparseHtmlInlines(n.data, True)
            s += '</span>'
        elif n.kind == 'Issue':
            url = 'http://code.google.com/p/grit/issues/detail?id=%d' % n.id
            s += '<a class="issue" href="%s">issue %d</a>' % (url, n.id)
        elif n.kind == 'Todo':
            s += '<span class="todo">'
            s += UnparseHtmlInlines(n.data, True)
            s += '</span>'
        elif n.kind == 'Web':
            s += '<a class="web" href="' + n.url + '">'
            s += UnparseHtmlInlines(n.data, True)
            s += '</a>'
        elif n.kind == 'SRef':
            the_file = GetOutputFile(n.target)
            if the_file.id == n.target.id:
                link = '%s.html' % the_file.id
            else:
                link = '%s.html#%s' % (the_file.id, n.target.id)
            s += '<a class="index" href="%s">' % link
            s += UnparseHtmlInlines(n.data, True)
            s += '</a>'
        else:
            print "ERROR: unknown node kind: " + n.kind
    return s


def UnparseHtmlBlocks(book, parent, path, section_counter, split_below, never_split):
    s = ""
    for n in parent:
        if n.kind == 'Image':
            s += '''<div class="image">
    <div class="image-title">''' + escape(n.title) + '''</div>
    <a class="image-link" href="''' + escape(n.src) + '''">
        <img class="thumbnail" src="''' + escape(n.thumb_src) + '''" />
    </a>
    <div class="image-caption">''' + escape(n.caption) + '''</div>
</div>\n\n'''
        elif n.kind == 'UnorderedList':
            s += '<ul>\n\n'
            for item in n.data:
                s += '<li>\n\n'
                s += UnparseHtmlBlocks(book, item, path, section_counter, split_below, never_split)
                s += '</li>\n\n'
            s += '</ul>\n\n'
        elif n.kind == 'Section':
            section_counter += 1
            new_path = path + [section_counter] 
            path_string = '.'.join([str(e) for e in new_path])
            h = len(new_path)
            inner_html = '<h%d id="%s">%s. %s</h%d>\n\n' % (h, n.id, path_string, escape(n.title), h)
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
            s += '<pre>%s</pre>\n\n' % n.data
        elif n.kind == 'Lua':
            s += '<pre class="lua">%s</pre>\n\n' % n.data
        elif n.kind == 'Paragraph':
            s += '<p>\n'
            s += '\n'.join(textwrap.wrap(UnparseHtmlInlines(n.data)))
            s += '\n</p>\n\n'
        else:
            print "ERROR: unknown node kind: " + n.kind
    if split_below and not never_split:
        s += BuildHtmlIndex(parent, path)
    return s


def BuildHtmlIndex(parent, path, indent=0, filename=None):
    indentstr = ' ' * (indent*4)
    index = indentstr + '<ul>\n'
    section_index = 0
    for n in parent:
        if n.kind == 'Section':
            section_index += 1
            new_path = path + [section_index]
            path_string = '.'.join([str(e) for e in new_path])
            new_filename = n.id + '.html' if parent.split else filename
            title = escape(n.title)
            index += ' ' * ((indent+1)*4)
            if parent.split:
                index += '<li class="index">%s. <a class="index" href="%s">%s</a></li>\n' % (path_string, new_filename, title)
            else:
                url = '%s#%s' % (new_filename, n.id)
                index += '<li class="index">%s. %s (<a class="index" href="%s">&sect;</a>)</li>\n' % (path_string, title, url)
            index += BuildHtmlIndex(n, new_path, indent+1, new_filename)
    index += indentstr + '</ul>\n'
    if section_index == 0:
        return ''
    return index

