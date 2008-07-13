#
# Creates installation files for tMan
# David Andrs, (c) 2006-2007
#

# Version
product="tMan"
version="0.9.10"

#
# Make installation files
#

# Build std. PPC2003 version (English)
rm lang/default.lng
"c:/program files/windows ce tools/wce420/pocket pc 2003/tools/cabwiz.exe" inf/$product.inf /cpu ARM

sleep 2  # wait for finish
cp inf/$product.ARM.CAB exe/$product.ARM.cab
# build exe
"c:/program files/nsis/makensis.exe" /DLANGUAGE=English exe/setup.nsi 
sleep 3

#exit
# move to destionation
mv -f exe/$product.ARM.cab export/$product-$version.ARM.cab
mv -f exe/$product.exe export/$product-$version.exe

# zip the cab
cd export
rm $product-$version.zip
zip -m $product-$version.zip $product-$version.ARM.cab
cd ..

# clean up
rm lang/default.lng

