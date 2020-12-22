
CMSHTTP is a simple web server written in GCC "C" that runs in a
VM370 CMS workspace.  This is a simple start with a lot more to 
be added. 

Questions/Comments:
tchandler48@gmail.com


IT DOES NOT SUPPORT SSL !!!!!



CMSHTTP Design Notes


HTTP/1.1 Based


FUNCTIONS:

	GET


SUPPORTTED WEB TYPES:


	html
	htm
       static url's
       relative url's


Web Tags NOT Suportted:


Limitations:

	1.	The maximum line length on a web page is 512.  Any characters
		beyond that will be dropped.

	2.	Sub directories are NOT allowed.  All web material must be
		in the root directory along with the CMSHTTP executables.

	3.	include files in a 1st level webpage CAN NOT be nested more
		than one level.  An included file CAN NOT include another 
		level of file.
			0 - index.html
			1 - 	page1.html		(OK)

			0 - index.html	
			1 - 	page1.html
			2 - 		page2.html	(NO beyond 1 level)

			0 - index.html
			1 -	page1.html
			1 - 	page2.html		(OK all includes are
							 on the 1st level)


