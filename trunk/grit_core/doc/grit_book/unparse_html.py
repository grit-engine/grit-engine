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

def GetParentIndex(node):
    i = 0
    for c in node.parent:
        if c.kind == 'Section':
            i += 1
        if c == node:
            return i
    return None
        

def GetPath(node):
    if not node.parent:
        return []
    here = []
    if node.kind == 'Section':
        here = [str(GetParentIndex(node))]
    return GetPath(node.parent) + here

def GetPathString(node):
    return '.'.join(GetPath(node))

def GetLinkToNode(target):
    the_file = GetOutputFile(target)
    if the_file.id == target.id:
        return '%s.html' % the_file.id
    else:
        return '%s.html#%s' % (the_file.id, target.id)

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
            s += '(<span class="todo">TODO: %s</span>)' % UnparseHtmlInlines(n.data, True)
        elif n.kind == 'Web':
            s += '<a class="web" href="' + n.url + '">'
            s += UnparseHtmlInlines(n.data, True)
            s += '</a>'
        elif n.kind == 'SRef':
            content = UnparseHtmlInlines(n.data, True)
            s += '%s (<a class="index" href="%s">&sect;%s</a>)' % (content, GetLinkToNode(n.target), GetPathString(n.target))
        else:
            print 'ERROR: unknown node kind: ' + n.kind
    return s


def UnparseHtmlBlocks(book, parent, split_below, never_split):
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
                s += UnparseHtmlBlocks(book, item, split_below, never_split)
                s += '</li>\n\n'
            s += '</ul>\n\n'
        elif n.kind == 'Section':
            new_path = GetPath(n)
            path_string = '.'.join(new_path)
            h = len(new_path)
            title = '<a class=index_nocol href="%s">%s. %s</a>' % (GetLinkToNode(n), path_string, escape(n.title))
            inner_html = '<h%d id="%s">%s</h%d>\n\n' % (h, n.id, title, h)
            inner_html += UnparseHtmlBlocks(book, n, n.split, never_split)
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
        s += BuildHtmlIndex(parent)
    return s


def BuildHtmlIndex(parent, indent=0):
    indentstr = ' ' * (indent*4)
    index = indentstr + '<ul>\n'
    section_index = 0
    for n in parent:
        if n.kind == 'Section':
            section_index += 1
            path_string = GetPathString(n)
            title = escape(n.title)
            index += ' ' * ((indent+1)*4)
            url = GetLinkToNode(n)
            cls = 'index' if parent.split else 'index_nocol'
            index += '<li class="index">%s. <a class="%s" href="%s">%s</a></li>\n' % (path_string, cls, url, title)
            index += BuildHtmlIndex(n, indent+1)
    index += indentstr + '</ul>\n'
    if section_index == 0:
        return ''
    return index

