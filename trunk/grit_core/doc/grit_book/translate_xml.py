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
     

def AssertExists(el, attr):
    v = el.get(attr)
    if not v:
        Error(el, 'Tag: "%s" needs attribute "%s".' % (el.tag, attr))
    return v
     

def AssertTag(el, tag):
    if el.tag != tag:
        Error(el, 'Expected %s, not %s.' % (tag, el.tag))
     

def AssertFile(el, fname):
    if not os.path.isfile(fname):
        Error(el, "File does not exist: " + fname)


class Node:
  def __init__(self, kind, parent, **kwargs):
    self.kind = kind
    self.parent = parent
    self.attr = kwargs
  def __getattr__(self, key):
    return self.attr.get(key)
  def __str__(self):
    return repr(self)
  def __repr__(self):
    return 'Node("%s", attr=%s)' % (self.kind, repr(self.attr))
  def __iter__(self):
    if self.data:
        for a in self.data:
            yield a
  def __len__(self):
    return len(self.data)
  def __nonzero__(self):
    return True


inline_tags = { 'def', 'web', 'issue', 'todo', 'sref' }

def MinimiseWhiteSpace(n):
    """Convert any whitespace to a single space."""
    return re.sub('[ \t\n]+', ' ', n)

def StripParagraphWhiteSpace(p, inside=False):
    """At the root of a paragraph, strip leading whitespace from the first
    element, and trailing whitespace from the last.  Minimise whitespace in all
    other cases.

    Args:
      p: List of strings and tags.
      inside: Whether or not we are recursing within the body of a tag within the paragraph.
    Returns:
      The list of strings and tags with maybe less whitespace.
    """
    r = []
    for i, n in enumerate(p):
        if isinstance(n, basestring):
            n = MinimiseWhiteSpace(n)
            if not inside:
                if i==0:
                    n = n.lstrip(' \t\n')
                if i==len(p)-1:
                    n = n.rstrip(' \t\n')
        elif n.data:
            n.data = StripParagraphWhiteSpace(n.data, True)
        r.append(n)
    return r
        

def NonEmptyParagraph(para):
    """A paragraph is non-empty if it contains more than just whitespace."""
    for el in para:
        if not isinstance(el, basestring):
            return True
        if el.strip('\n\t'):
            return True
    return False

SECTION_MAP = {}

def TranslateParagraphs(content, context_ast, dosplit=True):
    """Translate text and XML elements into paragraphs by examining whitespace.

    Args:
      content: A list of text and tags.  The text may span several paragraphs
               (represented by blank lines).  
      context_ast:
      dosplit:  Whether or not to treat blank lines as paragraph dividers.

    Returns:
      A list of paragraphs, where each paragraph is given as a list of strings and nodes.
    """
    r = []
    r2 = None
    for i, c in enumerate(content):
        if isinstance(c, basestring):
            for i, s in enumerate(re.split('\n[ \t]*\n',c) if dosplit else [c]):
                if i==0:
                    # Could be after an inline block, therefore not the beginning of a paragraph.
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
            content2 = TranslateParagraphs(([c.text] if c.text else []) + [ch for ch in c], context_ast, False)
            flattened =  [item for sublist in content2 for item in sublist]
            if c.tag == "def":
                r2.append(Node('Definition', context_ast, data=flattened))
            elif c.tag == "web":
                r2.append(Node('Web', context_ast, url=c.get('url'), data=flattened))
            elif c.tag == "issue":
                if flattened:
                    Error(c, "Tag: %s should not have textual content." % c.tag) 
                r2.append(Node('Issue', context_ast, id=int(c.get('id'))))
            elif c.tag == "todo":
                r2.append(Node('Todo', context_ast, data=flattened))
            elif c.tag == "sref":
                id = c.get('id')
                if not id:
                    Error(n, 'Tag: %s should have an id attribute.' % c.tag)
                target = SECTION_MAP.get(id)
                if not target:
                    Error(n, 'Referenced target does not exist: "%s"' % n.id)
                r2.append(Node('SRef', context_ast, target=target, data=flattened))
            else:
                Error(c, 'Unknown tag: ' + str(c.tag))
    r = map(StripParagraphWhiteSpace, r)
    r = filter(NonEmptyParagraph, r)
    return r


def TranslateBlockContents(block, block_ast):
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
            paragraphs = []
            for inline in TranslateParagraphs(el, None):
                n = Node('Paragraph', block_ast, data=inline)
                for i in inline:
                    if not isinstance(i, basestring):
                        i.parent = n
                paragraphs.append(n)
            output_content += paragraphs
        else:
            if el.tag == "section":
                id = AssertExists(el, 'id')
                sb = el.get('splitbelow')
                # Add Section node without data field first.
                translated_content = Node('Section', block_ast, split=sb=="true", id=id,
                                          title=el.get('title'), data=False)
                SECTION_MAP[id] = translated_content
                translated_content.data = TranslateBlockContents(el, translated_content)
                AssertNoTail(el)
            elif el.tag == "image":
                src = el.get('src')
                thumb_src = 'thumb_' + src
                caption = MinimiseWhiteSpace(el.text or '')
                title = MinimiseWhiteSpace(el.get('title'))
                AssertFile(el, src)
                AssertFile(el, thumb_src)
                translated_content = Node('Image', block_ast, src=src, caption=caption, thumb_src=thumb_src, title=title)
            elif el.tag == "ul":
                AssertNoBody(el)
                translated_items = []
                translated_content = Node('UnorderedList', block_ast)
                for item in el:
                    AssertTag(item, 'li')
                    translated_items.append(TranslateBlockContents(item, translated_content))
                    AssertNoTail(item)
                translated_content.data = translated_items
            elif el.tag == "lua":
                translated_content = Node('Lua', block_ast, data=textwrap.dedent(el.text))
            elif el.tag == "pre":
                translated_content = Node('Preformatted', block_ast, data=textwrap.dedent(el.text))
            else:
                Error(el, 'Unknown tag: ' + str(el.tag))
            output_content.append(translated_content)

    return output_content


def GetOutputFile(node):
    """Return the node that begins the current file, by chasing parents."""
    if not node.parent:
        raise 'Got to the bottom of the DOM tree without finding an output file.'
    if node.parent.split:
        return node
    return GetOutputFile(node.parent)
