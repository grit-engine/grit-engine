#!/usr/bin/python

import lxml.etree as ET

parser = ET.XMLParser(load_dtd=True, resolve_entities=True)

tree = ET.parse("index.xml", parser=parser)
tree.xinclude()

book = tree.getroot()

def handle_text(text):
    if not text:
        return
    paras = ['']
    for line in text.split('\n'):
        line = line.strip()
        if line == '':
            if paras[-1]:
                paras.append("")
        else:
            if paras[-1]:
                paras[-1] += ' '
            paras[-1] += line
    for para in paras:
        if para:
            print(para+'\n')

for chapter in book:
    assert chapter.tag == 'chapter'
    print '='*80 
    print(chapter.get('title'))
    print '='*80 
    print
    handle_text(chapter.text)
    for el in chapter:
        if el.tag == 'image':
            print '[IMAGE] '+el.get('title')
            print el.get('src')
            print el.get('caption')
            print
        else:
            assert False
        handle_text(el.tail)

