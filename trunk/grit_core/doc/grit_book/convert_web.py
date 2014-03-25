#!/usr/bin/python

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
    for filename, ast in pages:
        s += '\n<div class="navitem"><a class="navitem" href="' + filename + '">' + ast.title + '</a></div>'
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


print 'Parsing XML...'
parser = ET.XMLParser(load_dtd=True, resolve_entities=True)
tree = ET.parse('index.xml', parser=parser)
tree.xinclude()
book = tree.getroot()

print 'Translating XML dom...'
pages = []
for chapter in book:
    if chapter.tag == ET.Comment:
        continue
    assert chapter.tag == 'chapter'
    title = chapter.get('title')
    filename = chapter.get('filename')
    ast = Node('Section', title=title, data=TranslateBlockContents(chapter))
    pages.append((filename, ast))

print 'Unparsing to HTML...'
for filename, ast in pages:
    # Convert the chapter tag into a section, for easy translation.
    f = open(filename, 'w')
    f.write(GeneratePage(ast.title, UnparseHtmlBlocks([ast]), pages))
    f.close()

print 'Creating contents page...'
index = '<h1>Contents</h1>\n\n<ol>'
for filename, ast in pages:
    index += ('<li><p><a href="' + filename + '">' + ast.title + '</a></p></li>\n\n')
index += '</ol>\n'
contents = open('index.html', 'w')
contents.write(GeneratePage('Contents', index, pages))
contents.close()
print 'Done.'
