# C51_security
GSM security system based on Siemens C51 cell phone

AvrStudio 4 and avrgcc used

Encodded as cp1251 for AvrStudio 4

## Converting to utf8:

* just a file

iconv -f cp1251 -t utf8 /user/titles.txt -o /user/titles.txt

* number of files

for file in *; do cat $file | iconv -cs -f koi8-r -t utf8 > $file.tmp; mv -f $file.tmp $file; done


