#!/usr/bin/python

import lxml.etree as ET

parser = ET.XMLParser(load_dtd=True, resolve_entities=True)

tree = ET.parse('index.xml', parser=parser)
tree.xinclude()

book = tree.getroot()

def handle_text(text):
    if not text:
        return ''
    paras = ['']
    for line in text.split('\n'):
        if line == '':
            if paras[-1]:
                paras.append('')
        else:
            if paras[-1]:
                paras[-1] += ' '
            paras[-1] += line
    s = ''
    for para in paras:
        if para:
            if s:
                s+= '</p>\n\n<p>'
            s += para
    return s

def handle_image(el, depth):
    title = el.get('title')
    src = el.get('src')
    caption = el.get('caption')
    thumb_src = 'thumb_' + src
    return '''<div class="image">
<div>''' + title + '''</div>
<a href="''' + src + '''"><img src="''' + thumb_src + '''" /></a>
<div>''' + caption + '''</div>
</div>

'''

def handle_ul(el, depth):
    inner = '<ul>\n'
    assert not handle_text(el.text)
    for item in el:
        assert item.tag == 'li'
        inner += '\n<li>\n'
        inner += handle_text(item.text)
        inner += '\n</li>\n'
        assert not handle_text(item.tail)
    inner += '\n</ul>\n\n'
    return inner

inline_tags = { 'def' }

def handle_section(el, depth):
    return '<h%d>%s</h%d>\n\n%s' % (depth, el.get('title'), depth, handle_content(el, depth+1))

tags = {
    'def': (False, lambda el, d: '<span class="def">%s</span>' % el.text),
    'image': (True, handle_image),
    'section': (True, handle_section),
    'pre': (True, lambda el, d: '<pre>%s</pre>\n\n' % el.text.strip('\n')),
    'ul': (True, handle_ul),
}

def handle_content(content, depth):
    in_para = False
    s = ''
    before = handle_text(content.text)
    if before:
        s += '<p>' + before
        in_para = True

    for el in content:

        if el.tag == ET.Comment:
            pass
        else:
            handler = tags[el.tag]
            if not handler:
                print 'ERROR: Unknown tag: ' + str(el.tag)
                continue

            if handler[0]:
                if in_para:
                    s += '</p>\n\n'
                    in_para = False

            s += handler[1](el, depth)

            # FIXME: handle_text strips off final '\n' which is required to give leading space to the span
            after = handle_text(el.tail)
            print 'after: "%s"'%el.tail
            if after:
                if handler[0]:
                    s += '<p>'
                    in_para = True
                s += after

    if in_para:
        s += '</p>\n\n'

    return s


def page(title, content, chapters):
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
    for filename, title in chapters:
        s += '\n<div class="navitem"><a class="navitem" href="' + filename + '">' + title + '</a></div>'
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

chapters = []
for chapter in book:
    assert chapter.tag == 'chapter'
    title = chapter.get('title')
    assert title
    print('Processing chapter: '+title)
    filename = chapter.get('filename')
    assert filename
    chapters.append((filename, title))

for chapter in book:
    title = chapter.get('title')
    filename = chapter.get('filename')
    f = open(filename, 'w')
    content = '<h1>' + chapter.get('title') + '</h1>\n\n' + handle_content(chapter, 2)
    f.write(page(title, content, chapters))
    f.close()

index = ''
for chapter in book:
    filename = chapter.get('filename')
    title = chapter.get('title')
    index += ('<p><a href="' + filename + '">' + title + '</a></p>\n\n')


contents = open('index.html', 'w')
contents.write(page('Contents', index, chapters))
contents.close()
