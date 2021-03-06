<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiDoc to plain XHTML 1.1.
  -->
<xsl:transform
    version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    exclude-result-prefixes="xsl"
>

    <xsl:output
        method="text"
        encoding="us-ascii"
        indent="no"
    />

    <xsl:template match="/">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="/section">
        <xsl:apply-templates/>
        <xsl:text>&#xA;----&#xA;Warning: This wiki page is auto generated. If you change it, your changes are likely to be lost after the next update.&#xA;</xsl:text>
        <xsl:text>&#xA;Last modified: </xsl:text>
        <xsl:value-of select="current-dateTime()"/>
    </xsl:template>

    <xsl:template match="/section/title">
        <xsl:text>!!!</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="/section/section/title">
        <xsl:text>!!</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="/section/section/section/title">
        <xsl:text>!</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="/section/section/section/section/title">
        <xsl:text>__</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>__&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="blockcode">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="em">
        <xsl:text>''</xsl:text><xsl:apply-templates/><xsl:text>''</xsl:text>
    </xsl:template>

    <xsl:template match="strong">
        <xsl:text>__</xsl:text><xsl:apply-templates/><xsl:text>__</xsl:text>
    </xsl:template>

    <xsl:template match="ul/li">
        <xsl:text>*</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="ul/li/ul/li">
        <xsl:text>**</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="ul/li/ul/li/ul/li">
        <xsl:text>***</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="ol/li">
        <xsl:text>#</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="ol/li/ol/li">
        <xsl:text>##</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="ol/li/ol/li/ol/li">
        <xsl:text>###</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="dl/dt">
        <xsl:text>;</xsl:text><xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="dl/dd">
        <xsl:text>:</xsl:text><xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="p|section">
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="a">
        <xsl:text>[</xsl:text>
            <xsl:apply-templates/>
        <xsl:text> | </xsl:text><xsl:value-of select="@href"/><xsl:text>]</xsl:text>
    </xsl:template>

    <xsl:template match="table">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="tr">
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="td">
        <xsl:for-each select="for $i in 1 to @colspan return $i">
            <xsl:text>|</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="for $i in 2 to @rowspan return $i">
            <xsl:text>v</xsl:text>
        </xsl:for-each>
        <xsl:choose>
            <xsl:when test="@align='center'"><xsl:text>^</xsl:text></xsl:when>
            <xsl:when test="@align='right'"><xsl:text>&gt;</xsl:text></xsl:when>
            <!-- td default is left, phpWiki default is left, too -->
        </xsl:choose>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="th">
        <xsl:for-each select="for $i in 1 to @colspan return $i">
            <xsl:text>|</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="for $i in 2 to @rowspan return $i">
            <xsl:text>v</xsl:text>
        </xsl:for-each>
        <xsl:choose>
            <xsl:when test="@align='right'"><xsl:text>&gt;</xsl:text></xsl:when>
            <xsl:when test="@align='left'"><xsl:text>&lt;</xsl:text></xsl:when>
            <xsl:otherwise>^</xsl:otherwise>
            <!-- th default is center but phpWiki default is left -->
        </xsl:choose>
        <xsl:text>__</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>__</xsl:text>
    </xsl:template>

    <xsl:template match="br">
        <xsl:text>%%%</xsl:text>
    </xsl:template>

    <xsl:template match="*">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="blockcode/text()">
        <!-- Insert a single space at the beginning of lines. -->
        <xsl:value-of select="replace(replace(., '^', ' ', 'm'), '\[', '[[')"/>
    </xsl:template>

    <xsl:template match="text()">
        <xsl:if test="matches(.,'^\s')"><xsl:text> </xsl:text></xsl:if>
        <!-- Strip whitespace at the beginning of lines. -->
        <xsl:value-of select="replace(replace(normalize-space(), '^ +', '', 'm'), '\[', '[[')"/>
        <xsl:if test="matches(.,'\s$')"><xsl:text> </xsl:text></xsl:if>
    </xsl:template>

</xsl:transform>
