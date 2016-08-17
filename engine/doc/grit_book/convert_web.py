#!/usr/bin/python

import codecs
import re
from shutil import copyfile

import lxml.etree as ET
from translate_xml import *
from unparse_html import *
from unparse_markdown import *
from shutil import copyfile

# libxml2 does not properly implement the XML standard, so do my own implementation here...
def MyXInclude(tree):
    for n in tree:
        if n.tag == 'include':
            src = 'xml/' + n.get('src')
            print 'Parsing ' + src
            include_tree = ET.parse(src)
            include_tree.getroot().set('{http://www.w3.org/XML/1998/namespace}base', src)
            MyXInclude(include_tree.getroot())
            tree.replace(n, include_tree.getroot())
        else:
            MyXInclude(n)


_ID_REGEX = re.compile("^[_a-z0-9]+$")

ID_MAP = {}

# Ensure no two ids are the same
def CheckDupIds(node):
    nid = node.get('id')
    if node.tag == 'section':
        if not nid:
            Error(node, 'Section must have id attribute.')
        if not _ID_REGEX.match(nid):
            Error(node, 'Section id uses invalid characters.')
        existing = ID_MAP.get(nid)
        if existing is not None:
            Error(node, 'Section id already exists: %s.  The other exists at %s:%d' % (nid, existing.base, existing.sourceline))
        ID_MAP[nid] = node
    elif node.tag in ['issue', 'sref']:
        pass
    else:
        if nid:
            Error(node, 'Only section tags can have id attribute.')
    for child in node:
        CheckDupIds(child)

# Ensure ids are not dangling
def CheckRefIds(node):
    if node.tag == 'sref':
        nid = node.get('id')
        if not nid:
            Error(node, 'sref must have id attribute.')
        if ID_MAP.get(nid) is None:
            Error(node, 'Referenced section id does not exist: ' + nid)
    for child in node:
        CheckRefIds(child)


print 'Parsing xml/index.xml'
tree = ET.parse('xml/index.xml')
MyXInclude(tree.getroot())
book = tree.getroot()

CheckDupIds(book)
CheckRefIds(book)
AssertTag(book, 'book')

print 'Translating XML dom...'
book_ast = Node('Book', None, split=True, data=False)
book_ast.data = TranslateBlockContents(book, book_ast)
ResolveReferences(book_ast)

# Fetch all images
def GetImgSrcs(ast):
    if ast is None:
        return []
    if isinstance(ast, basestring):
        return []
    if isinstance(ast, list):
        return [s for child in ast for s in GetImgSrcs(child)]
    if ast.kind == 'Image':
        return [ast.src]
    return GetImgSrcs(ast.data)

all_imgs = GetImgSrcs(book_ast)
print all_imgs

# Web .html
print 'Writing html/index.html'
index = '<h1> Contents </h1>\n'
index += UnparseHtmlBlocks(book_ast, book_ast, True, False)
contents = codecs.open('html/index.html', 'w', 'utf-8')
contents.write(GeneratePage('Contents', index, book_ast))
contents.close()
pygments_css = codecs.open('html/pygments.css', 'w', 'utf-8')
pygments_css.write(HtmlFormatter().get_style_defs('.highlight'))
pygments_css.close()

print 'Writing html/complete.html'
index = UnparseHtmlBlocks(book_ast, book_ast, True, True)
contents = codecs.open('html/complete.html', 'w', 'utf-8')
contents.write(GeneratePage('&copy; 2016 Grit Engine Community.', index, None))
contents.close()

print 'Copyimg images to html'
for img in all_imgs:
    copyfile('xml/' + img, 'html/' + img)
    copyfile('xml/thumb_' + img, 'html/thumb_' + img)

print 'Copyimg images to md'
for img in all_imgs:
    copyfile('xml/' + img, 'md/' + img)

# Markdown .md
print 'Writing md/index.md'
index = '# Contents\n'
index += UnparseMarkdownBlocks(book_ast, book_ast, True, False)
contents = codecs.open('md/index.md', 'w', 'utf-8')
contents.write(UnparseMarkdownGeneratePage('Contents', index, book_ast))
contents.close()

print 'Writing md/complete.md'
index = UnparseMarkdownBlocks(book_ast, book_ast, True, True)
contents = codecs.open('md/complete.md', 'w', 'utf-8')
contents.write(UnparseMarkdownGeneratePage('Contents', index, None))
contents.close()

# Main README.md from Compiling Instruction
copyfile('md/README.md', '../../../README.md')

print 'Done.'
