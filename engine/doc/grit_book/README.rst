Grit Book
=========

This folder contains the sources of the `Grit Book
<http://www.gritengine.com/grit_book/>`_.

Dependencies
------------

You must install the following dependencies to build the Grit Book:

-   `HTMLDOC <https://www.msweet.org/projects.php?Z1>`_

-   `Markdown <http://daringfireball.net/projects/markdown/>`_

-   `Python 2.x <https://www.python.org/downloads/>`_ and the following Python
    libraries:

    -   `lxml <http://lxml.de/>`_

    -   `Pygments <http://pygments.org/>`_


Building the Grit Book
----------------------

To build the Grit Book, run::

    ./generate_doc.sh

The book is generated inside :code:`html/`. You can open
:code:`html/index.html` in a web browser to read the book::

    xdg-open html/index.html
