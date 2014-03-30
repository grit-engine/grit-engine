#!/usr/bin/python

import codecs

import lxml.etree as ET
from translate_xml import *
from unparse_http import *



# libxml2 does not properly implement the XML standard, so do my own implementation here...
def MyXInclude(tree):
    for n in tree:
        if n.tag == 'include':
            src = n.get('src')
            print 'Parsing ' + src
            include_tree = ET.parse(src)
            include_tree.getroot().set('{http://www.w3.org/XML/1998/namespace}base', src)
            tree.replace(n, include_tree.getroot())
        else:
            MyXInclude(n)

id_map = {}
# Ensure no two ids are the same
def CheckIds(node):
    if node.tag == 'section':
        nid = node.get('id')
        if not nid:
            Error(node, 'Section must have id attribute.')
        if id_map[nid]:
            Error(node, 'Section id already exists: ' + nid)
        id_map[nid] = n
    for child in node:
        CheckIds(child)
            

print 'Parsing index.xml'
tree = ET.parse('index.xml')
MyXInclude(tree.getroot())
book = tree.getroot()

AssertTag(book, 'book')

print 'Translating XML dom...'
book_ast = Node('Book', split=True, data=TranslateBlockContents(book))

print 'Writing index.html'
index = '<h1> Contents </h1>\n'
index += UnparseHtmlBlocks(book_ast, book_ast, [], 0, True, False)
contents = codecs.open('index.html', 'w', 'utf-8')
contents.write(GeneratePage('Contents', index, book_ast))
contents.close()

print 'Writing complete.html'
index = UnparseHtmlBlocks(book_ast, book_ast, [], 0, True, True)
contents = codecs.open('complete.html', 'w', 'utf-8')
contents.write(GeneratePage('Contents', index, None))
contents.close()

print 'Done.'
