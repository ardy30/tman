#
# Creates installation files for Birthdays
# David Andrs, (c) 2005
#

# Version
version="0.9.3-beta1"
product="tMan"

#
# Make installation files
#

# Build std. PPC2003 version (English)
#"c:/program files/windows ce tools/wce420/pocket pc 2003/tools/cabwiz.exe" inf/$product.inf /cpu ARM
"C:/Program Files/Windows CE Tools/wce300/Pocket PC 2002/support/ActiveSync/windows ce application installation/cabwiz/cabwiz.exe" inf/tMan.inf /cpu ARM.PPC2002

sleep 2  # wait for finish
cp inf/$product.ARM.PPC2002.CAB exe/$product.ARM.cab
# build exe
"c:/program files/nsis/makensis.exe" /DLANGUAGE=English exe/setup.nsi 
sleep 3

# move to destionation
mv -f exe/$product.ARM.cab export/$product-$version.ARM.cab
mv -f exe/$product.exe export/$product-$version.exe

# zip the cab
cd export
rm $product-$version.zip
zip -m $product-$version.zip $product-$version.ARM.cab
cd ..



