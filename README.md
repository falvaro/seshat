SESHAT: Handwritten math expression parser
==========================================
*Seshat* is an open-source system for recognizing handwritten
mathematical expressions. Given a sample represented as a sequence of
strokes, the parser is able to convert it to LaTeX or other formats
like InkML or MathML. You can see an example of application of this
parser in

http://cat.prhlt.upv.es/mer/

where *seshat* is the underlying engine.

This parser has been developed by Francisco Álvaro as part of his PhD
thesis while he was a member of the [PRHLT research center] [1] at
[Universitat Politècnica de València] [2].

*Seshat* represents a state-of-the-art system that has participated in
several [international competitions] [3], and it was awarded the best
system trained on the competition dataset in:

- Mouchère H., Viard-Gaudin C., Zanibbi R., Garain U.
  *ICFHR 2014 Competition on Recognition of On-line Handwritten 
   Mathematical Expressions (CROHME 2014)*.
  International Conference on Frontiers in Handwriting Recognition (ICFHR),
  Crete Island, Greece (2014)

The math expression recognition model that *seshat* implements is described in:

- Francisco Álvaro, Joan-Andreu Sánchez and José-Miguel Benedí.
  *An Integrated Grammar-based Approach for Mathematical Expression Recognition*.
  Pattern Recognition, pp. 135-147, 2016

and it is the main part of my PhD thesis. 

 - Francisco Álvaro (advisors: Joan-Andreu Sánchez and José-Miguel Benedí).
   [Mathematical Expression Recognition based on Probabilistic Grammars][13].
   Doctor of Philosophy in Computer Science,
   Universitat Politècnica de València, 2015.

License
-------
*Seshat* is released under the [GNU General Public License version 3.0 (GPLv3)] [5]


Distribution details
--------------------
*Seshat* is written in C++ and should work on any platform, although
it has only been tested in Linux.

This software integrates the open-source [RNNLIB library] [4]
for symbol classification. The code of RNNLIB has been slightly
modified and directly integrated in *seshat*, thus, it is not
necessary to download it. However, it requires the [Boost C++
Libraries] [6] (headers only).

Finally, the parser accepts input files in two formats: InkML and
SCGINK. There is a example of each format in folder
"SampleMathExps". *Seshat* uses the [Xerces-c library] [7] for parsing
InkML in C++.



Installation
--------------------
*Seshat* is written in C++ and it only requires Makefile and g++ to
compile it. Once the required tools and libraries are available, you
can proceed with the installation of *seshat* as follows:

 1. Obtain the package using git:

        $ git clone https://github.com/falvaro/seshat.git

    Or [download it as a zip file] [8]

 2. Go to the directory containing the source code.

 3. If the include files of boost libraries are not in the path, add
 it to the *FLAGS* variable in the file *Makefile* ("-I/path/to/boost/").

 4. Compile *seshat*

      $ make

As a result, you will have the executable file "*seshat*" ready to
recognize handwritten math expressions.


Example of usage
----------------
Run *seshat* without arguments and it will display the command-line interface:

```
$ Usage: ./seshat -c config -i input [-o output] [-r render.pgm]

  -c config: set the configuration file
  -i input:  set the input math expression file
  -o output: save recognized expression to 'output' file (InkML format)
  -r render: save in 'render' the image representing the input expression (PGM format)
  -d graph:  save in 'graph' the description of the recognized tree (DOT format)
```

There are two example math expressions in folder "SampleMathExps". The
following command will recognize the expression `(x+y)^2` encoded in
"exp.scgink"

	$ ./seshat -c Config/CONFIG -i SampleMathExps/exp.scgink -o out.inkml -r render.pgm -d out.dot

This command outputs several information through the standard output, where the last line will
provide the LaTeX string of the recognized math expression. Furthermore:

- An image representation of the input strokes will be rendered in "render.pgm".

- The InkML file of the recognized math expression will be saved in "out.inkml".

- The derivation tree of the expression provided as a graph in DOT
  format will be saved in "out.dot". The representation of the graph
  in, for example, postscript format can be obtained as follows

       	  $ dot -o out.ps out.dot -Tps

It should be noted that only options "-c" and "-i" are mandatory.


Citations
---------
If you use *seshat* for your research, please cite the following reference:

<pre>
@article{AlvaroPR16,
title = "An integrated grammar-based approach for mathematical expression recognition",
author = "Francisco \'Alvaro and Joan-Andreu S\'anchez and Jos\'e-Miguel Bened\'{\i}",
journal = "Pattern Recognition",
volume = "51",
pages = "135 - 147",
year = "2016",
issn = "0031-3203"
}
</pre>


Why *seshat*?
-------------
*Seshat* was the [Goddess of writing] [9] according to Egyptian
mythology, so I liked this name for a handwritten math expression
parser. I found out about *seshat* because my colleague of the PRHLT
[Daniel Ortiz-Martínez] [10] developed [Thot] [11], a great
open-source toolkit for statistical machine translation; and Thot is
the [God of Knowledge] [12] according to Egyptian mythology.




[1]: http://www.prhlt.upv.es/
[2]: http://www.upv.es/
[3]: http://www.isical.ac.in/~crohme/
[4]: http://sourceforge.net/projects/rnnl/
[5]: http://www.gnu.org/licenses/gpl-3.0.html
[6]: http://www.boost.org/
[7]: http://xerces.apache.org/xerces-c/
[8]: https://github.com/falvaro/seshat/archive/master.zip
[9]: http://en.wikipedia.org/wiki/Seshat
[10]: https://www.prhlt.upv.es/page/member?user=dortiz
[11]: https://github.com/daormar/thot
[12]: http://en.wikipedia.org/wiki/Thoth
[13]: http://hdl.handle.net/10251/51665
