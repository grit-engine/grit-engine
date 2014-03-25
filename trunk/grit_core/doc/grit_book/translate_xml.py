#!/usr/bin/python

import lxml.etree as ET

class Node:
  def __init__(self, kind, **kwargs):
    self.kind = kind
    self.attr = kwargs
  def __getattr__(self, key):
    return self.attr[key]
  def __repr__(self):
    return 'Node("%s", attr=%s)' % (self.kind, repr(self.attr))

inline_tags = { 'def', 'web', 'issue' }

def TranslateParagraphs(content, dosplit=True):
    r = []
    r2 = None
    for i, c in enumerate(content):
        if isinstance(c, str):
            for i, s in enumerate(c.split('\n\n') if dosplit else [c]):
                if i==0:
                    if not r2:
                        r2 = []
                        r.append(r2)
                else:
                    r2 = []
                    r.append(r2)
                r2.append(s)
        else:
            if not r2:
                r2 = []
                r.append(r2)
            content2 = TranslateParagraphs(([c.text] if c.text else []) + [ch for ch in c], False)
            flattened =  [item for sublist in content2 for item in sublist]
            if c.tag == "def":
                r2.append(Node('Definition', data=flattened))
            elif c.tag == "web":
                r2.append(Node('Web', url=c.get('url'), data=flattened))
            elif c.tag == "issue":
                assert not flattened
                r2.append(Node('Issue', id=int(c.get('id'))))
            else:
                print 'ERROR: Unknown tag: ' + str(c.tag)
    return r


def TranslateBlockContents(block):
    block_content = []
    text_content = None
    if block.text:
        text_content = [block.text]
        block_content.append(text_content)

    for el in block:
        if el.tag == ET.Comment:
            if el.tail:
                text_content = [el.tail]
                block_content.append(text_content)
        elif el.tag in inline_tags:
            if text_content:
                text_content.append(el)
            else:
                text_content = [el]
                block_content.append(text_content)
            if el.tail:
                text_content.append(el.tail)
        else:
            block_content.append(el)
            text_content = None
            if el.tail:
                text_content = [el.tail]
                block_content.append(text_content)

    output_content = []
    for el in block_content:
        if isinstance(el,list):
            output_content += [Node('Paragraph', data=inline) for inline in TranslateParagraphs(el)]
        else:
            if el.tag == "section":
                data = TranslateBlockContents(el)
                translated_content = Node('Section', index='2', title=el.get('title'), data=data)
            elif el.tag == "image":
                src = el.get('src')
                translated_content = Node('Image', src=src, caption=el.get('caption'),
                                          thumb_src='thumb_' + src, title=el.get('title'))
            elif el.tag == "ul":
                assert not el.text.strip('\n\t ')
                translated_items = []
                for item in el:
                    assert item.tag == 'li'
                    translated_items.append(TranslateBlockContents(item))
                    assert not item.tail.strip('\n\t ')
                translated_content = Node('UnorderedList', data=translated_items)
            elif el.tag == "lua":
                translated_content = Node('Lua', data=el.text.strip('\n'))
            elif el.tag == "pre":
                translated_content = Node('Preformatted', data=el.text.strip('\n'))
            else:
                print 'ERROR: Unknown tag: ' + str(el.tag)
                continue
            output_content.append(translated_content)

    return output_content
