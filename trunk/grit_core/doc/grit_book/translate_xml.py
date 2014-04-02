#!/usr/bin/python

import os.path
import re
import textwrap

import lxml.etree as ET

class TranslateError:
    def __init__(self, el, problem):
        self.filename = el.base
        self.line = el.sourceline
        self.problem = problem
    def __str__(self):
        return '%s:%d: %s' % (self.filename, self.line, self.problem)


def Error(el, msg):
    raise TranslateError(el, msg)


def AssertNoBody(el):
    if el.text and el.text.strip('\n\t '):
        Error(el, 'Cannot have text after element %s.' % el.tag)


def AssertNoTail(el):
    if el.tail and el.tail.strip('\n\t '):
        Error(el, 'Cannot have text after element %s.' % el.tag)
     

def AssertTag(el, tag):
    if el.tag != tag:
        Error(el, 'Expected %s, not %s.' % (tag, el.tag))
     

def AssertFile(el, fname):
    if not os.path.isfile(fname):
        Error(el, "File does not exist: " + fname)


class Node:
  def __init__(self, kind, **kwargs):
    self.kind = kind
    self.attr = kwargs
  def __getattr__(self, key):
    return self.attr.get(key)
  def __str__(self):
    return repr(self)
  def __repr__(self):
    return 'Node("%s", attr=%s)' % (self.kind, repr(self.attr))
  def __iter__(self):
    for a in self.data:
        yield a
  def __len__(self):
    return len(self.data)
  def __nonzero__(self):
    return True


inline_tags = { 'def', 'web', 'issue', 'todo' }

def MinimiseWhiteSpace(n):
    return re.sub('[ \t\n]+', ' ', n)

def StripParagraphWhiteSpace(p, inside=False):
    # Strip leading and whitespcae from the first element, and trailing from
    # the last.  Minimise whitespace in all other cases.
    r = []
    for i, n in enumerate(p):
        if isinstance(n, basestring):
            n = MinimiseWhiteSpace(n)
            if i==0 and not inside:
                n = n.lstrip(' \t\n')
            if i==len(p)-1 and not inside:
                n = n.rstrip(' \t\n')
        elif n.data:
            n.data = StripParagraphWhiteSpace(n.data, True)
        r.append(n)
    return r
        

def NonEmptyParagraph(p):
    if len(p) > 1:
        return True
    if p[0].strip('\n\t '):
        return True
    return False


# FIXME: paragraphs divided by empty lines containing whitespace are not split
def TranslateParagraphs(content, dosplit=True):
    r = []
    r2 = None
    for i, c in enumerate(content):
        if isinstance(c, basestring):
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
                if flattened:
                    Error(c, "Issue tag should be empty.") 
                r2.append(Node('Issue', id=int(c.get('id'))))
            elif c.tag == "todo":
                r2.append(Node('Todo', data=flattened))
            else:
                Error(c, 'Unknown tag: ' + str(c.tag))
    r = map(StripParagraphWhiteSpace, r)
    r = filter(NonEmptyParagraph, r)
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
                sb = el.get('splitbelow')
                translated_content = Node('Section', split=sb=="true", id=el.get('id'),
                                          title=el.get('title'), data=data)
                AssertNoTail(el)
            elif el.tag == "image":
                src = el.get('src')
                thumb_src = 'thumb_' + src
                caption = MinimiseWhiteSpace(el.get('caption'))
                title = MinimiseWhiteSpace(el.get('title'))
                AssertFile(el, src)
                AssertFile(el, thumb_src)
                translated_content = Node('Image', src=src, caption=caption, thumb_src=thumb_src, title=title)
            elif el.tag == "ul":
                AssertNoBody(el)
                translated_items = []
                for item in el:
                    AssertTag(item, 'li')
                    translated_items.append(TranslateBlockContents(item))
                    AssertNoTail(item)
                translated_content = Node('UnorderedList', data=translated_items)
            elif el.tag == "lua":
                translated_content = Node('Lua', data=textwrap.dedent(el.text))
            elif el.tag == "pre":
                translated_content = Node('Preformatted', data=textwrap.dedent(el.text))
            else:
                Error(el, 'Unknown tag: ' + str(el.tag))
            output_content.append(translated_content)

    return output_content
