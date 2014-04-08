# Documentation #
Documentation files uses the GitHub markdown syntax. A common file extension for markdown files is .md.


[GitHub Markdown syntax](https://help.github.com/articles/github-flavored-markdown)

## How to produce PDF from markdown files ##

Pandoc can be used to generated PDF from a markdown file like this

    pandoc -f markdown_github GettingStarted.md -o GettingStarted.pdf

See here for information on Pandoc
[http://johnmacfarlane.net/pandoc/](http://johnmacfarlane.net/pandoc/)


## Internal documentation notes

### Produce PDF from markup - 28. oct 2013

The documentation for Git is build using a set of markup files. Below is a set of command history on Ubuntu 13.04 to be able to produce PDF from markup.

NOTE: On Ubuntu, use the following fonts in latex/config.yml
Ubuntu for normal text and DejaVu Sans Mono for monospace. 

https://github.com/progit/progit/issues/11


    16  cd ..
    17  git clone https://github.com/progit/progit.git
    18  cd progit/
    19  ls
    20  ./makepdfs en
    21  sudo apt-get install ruby
    22  ./makepdfs en
    23  sudo apt-get install pandoc
    24  ./makepdfs en
    25  sudo apt-get install xelatex
    26  sudo apt-get install texlive-xetex
    27  ./makepdfs en
    28  dir
    29  sudo apt-get install texlive
    30  ./makepdfs en
    31  sudo apt-get install texlive-latex-extra
    32  sudo apt-get install texlive-latex-base
    33  ./makepdfs en
    34  sudo apt-get install texlive-latex-extra
    35  ./makepdfs en
    36  ls
    37  cd latex/
    38  ls
    39  kate config.yml 
    40  sudo apt-get install kate
    41  kedit config.yml 
    42  gedit config.yml 
    43* 
    44  cd ..
    45  ls
    46  ./makepdfs en
    47  fc-match helvetica
    48  ./makepdfs en
    49  fc-list
    50  fc-match helvetica
    51  fc-match helvetica neue
    52  ./makepdfs en
    53  fc-list
    54  ./makepdfs en
    55  ./makepdfs en --debug
    56  ./makepdfs en
    57  ls
    58  ./progit.en.pdf
    59  evince
    60  history
