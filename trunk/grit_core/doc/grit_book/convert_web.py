#!/usr/bin/python

import codecs

import lxml.etree as ET
from translate_xml import *
from unparse_http import *


def GeneratePage(title, content, pages):
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
    s += '\n<div class="navitem"><a class="navitem" href="index.html">Contents</a></div>'
    chapter_index = 0
    for filename, ast in pages:
        chapter_index += 1
        s += '\n<div class="navitem"><a class="navitem" href="%s">%d. %s</a></div>' % (filename, chapter_index, ast.title)
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


# libxml2 does not properly implement the XML standard, so do my own implementation here...
def MyXInclude(tree):
    for n in tree:
        if n.tag == '{http://www.w3.org/2001/XInclude}include':
            href = n.get('href')
            print 'Parsing ' + href
            include_tree = ET.parse(href)
            include_tree.getroot().set('{http://www.w3.org/XML/1998/namespace}base', href)
            tree.replace(n, include_tree.getroot())
        else:
            MyXInclude(n)
            

print 'Parsing index.xml'
tree = ET.parse('index.xml')
MyXInclude(tree.getroot())
book = tree.getroot()


print 'Translating XML dom...'
pages = []
AssertTag(book, 'book')
for chapter in book:
    if chapter.tag == ET.Comment:
        continue
    AssertTag(chapter, 'chapter')
    title = chapter.get('title')
    id = chapter.get('id')
    filename = id + '.html'
    ast = Node('Section', id=id, title=title, data=TranslateBlockContents(chapter))
    pages.append((filename, ast))

chapter_index = 0
for filename, ast in pages:
    # Convert the chapter tag into a section, for easy translation.
    print 'Unparsing ' + filename
    f = codecs.open(filename, 'w', 'utf-8')
    f.write(GeneratePage(ast.title, UnparseHtmlBlocks([ast], [], chapter_index), pages))
    chapter_index += 1
    f.close()

def Sections(ast, filename, path):
    index = ''
    section_index = 0
    for n in ast:
        if n.kind == 'Section':
            section_index += 1
            new_path = path + [section_index]
            path_string = '.'.join([str(e) for e in new_path])
            indent = '&nbsp;' * (len(path) * 8)
            url = '%s#%s' % (filename, n.id)
            index += '<p>%s%s <a href="%s">%s</a></p>\n\n' % (indent, path_string, url, n.title)
            index += Sections(n, filename, new_path)
    return index

print 'Writing index.html'
index = '<h1>Contents</h1>\n\n'
chapter_index = 0
for filename, ast in pages:
    chapter_index += 1
    index += '<p>%d <a href="%s">%s</a></p>\n\n' % (chapter_index, filename, ast.title)
    index += Sections(ast, filename, [chapter_index])
index += '\n'
contents = codecs.open('index.html', 'w', 'utf-8')
contents.write(GeneratePage('Contents', index, pages))
contents.close()
print 'Done.'
