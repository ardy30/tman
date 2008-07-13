#
# Creates installation files
# David Andrs, (c) 2005
#

product=tMan
version="0.9.10"

#lang[1]="czech cs Czech"
#lang[2]="german de German"
#lang[3]="italian it Italian"
#lang[4]="russian ru Russian"
#lang[5]="spanish es Spanish"
#lang[6]="dutch nl Dutch"
#lang[7]="french fr French"
#lang[8]="polish pl Polish"
#lang[9]="macedonian mk Macedonian"
#lang[10]="portuguese pt Portuguese"
#lang[11]="turkish tr Turkish"
#lang[12]="hungarian hu Hungarian"
#lang[13]="thai th Thai"
#lang[14]="ukrainian ua Ukrainian"
#lang[15]="slovak sk Slovak"
#lang[15]="croatian hr Croatian"
#lang[16]="norwegian no Norwegian"
#lang[17]="hebrew he Hebrew"

#
# Make installation files
#

rm lang/default.lng

# Build localized versions
for l in $lang; do 
	fn=`echo $l | cut -d \  -f 1`
	code=`echo $l | cut -d \  -f 2`
	slng=`echo $l | cut -d \  -f 3`

	echo $code
	
	# Move language file to lang/default (installation script is awaiting it here)
	cp $PDA_LOCAL/tman/lang/$fn.lng lang/default.lng
	# build localized version
	"c:/program files/windows ce tools/wce420/pocket pc 2003/tools/cabwiz.exe" inf/$product.inf /cpu ARM.Localized
	sleep 2  # wait for finish

	cp inf/$product.ARM.CAB exe/$product.ARM.cab
	# build exe
	"c:/program files/nsis/makensis.exe" /DLANGUAGE=$slng exe/setup.nsi 
	sleep 3

	# move to destionation
	mv -f inf/$product.ARM.Localized.cab export/$product-$version-$code.ARM.cab
	mv -f exe/$product.exe export/$product-$version-$code.exe

	# zip the cab
	cd export
	rm $product-$version-$code.zip
	zip -m $product-$version-$code.zip $product-$version-$code.ARM.cab
	cd ..
done

# clean up
rm lang/default.lng

