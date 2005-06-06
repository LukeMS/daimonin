<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiDoc to plain XHTML 1.1.
  -->
<xsl:transform
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    exclude-result-prefixes="xsl"
>

    <xsl:strip-space elements="*" />
    <xsl:preserve-space elements="blockcode" />

    <xsl:output
        method="text"
        output-encoding="us-ascii"
        indent="no"
    />

    <xsl:template match="/section/title">!!!<xsl:apply-templates/><xsl:text>&#xA;</xsl:text></xsl:template>

    <xsl:template match="/section/section/title">!!<xsl:apply-templates/><xsl:text>&#xA;</xsl:text></xsl:template>

    <xsl:template match="/section/section/section/title">!<xsl:apply-templates/><xsl:text>&#xA;</xsl:text></xsl:template>

    <xsl:template match="blockcode">
        <!-- TODO: indent every line by 1 space -->
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
    </xsl:template>

    <xsl:template match="p|section">
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="*">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="blockcode/text()">
        <xsl:value-of select="."/>
    </xsl:template>

    <xsl:template match="text()">
        <xsl:value-of select="normalize-space(.)"/>
    </xsl:template>

</xsl:transform>
