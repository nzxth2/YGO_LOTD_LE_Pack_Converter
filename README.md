# YGO LOTD Link Evolution Pack Converter
 Converter for .bin pack files of Yu-Gi-Oh! Legacy of the Duelist Link Evolution 

## Features
 - Converts .bin pack files to humanly readable .txt files
 - Converts said .txt files back to .bin files
 - Allows for modification of existing packs
 - Does not yet work for Battle Packs (Draft)
 - If a card in a .txt file can not be identified, it will be recorded in missingCards.txt
 - Check the /extra directory in the repository for a mass-converted list of all packs
  
 ### TXT File Format
 ```
 # common cards. first the card count, then the list
 248
 CardName1
 .
 .
 .
 CardName248
 # rare cards. first the card count, then the list
 66
 CardName1
 .
 .
 .
 CardName66
 ```
  
 Note that comments start with `#` and are always ignored, as are empty lines
 
## Building
  Not crossplatform at all, only builds for Windows  
  Included the makefile used with gcc, a simple `make` should do the trick  
  No external libraries should be required

## Credits
 Developed by nzxth2  
 [Nuklear GUI library](https://github.com/Immediate-Mode-UI/Nuklear) developed by Micha Mettke and others  
 [Notes](https://github.com/MoonlitDeath/Legacy-of-the-Duelist-notes/wiki) and [guide](https://github.com/MoonlitDeath/Link-Evolution-Editing-Guide/wiki) by MoonlitDeath

## Gallery
 ![Screenshot](https://i.imgur.com/hKpsnQZ.png)