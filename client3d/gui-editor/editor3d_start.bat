@echo off

java -classpath editor3d.jar;lib/commons-collections-3.2.jar;lib/commons-configuration-1.1.jar;lib/commons-lang-2.1.jar;lib/commons-logging-1.1.jar;lib/dom4j-1.6.1.jar;lib/jai_core.jar;lib/jai_codec.jar net.daimonin.client3d.editor.main.Editor3D %1 %2
