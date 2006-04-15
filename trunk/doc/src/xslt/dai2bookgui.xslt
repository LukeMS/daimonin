<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiML to the Daimonin BookGUI format.
  -->
<xsl:transform
    version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    exclude-result-prefixes="xsl"
>

    <xsl:strip-space elements="*" />
    <xsl:preserve-space elements="blockcode" />

    <xsl:output
        method="text"
        encoding="utf-8"
        indent="no"
    />

    <xsl:template match="/daiml">
        <xsl:text>&lt;bt="</xsl:text>
        <xsl:value-of select="@title"/>
        <xsl:text>"&gt;&#xa;</xsl:text>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="/daiml/section">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="/daiml/section/title">
        <xsl:text>&lt;pt="</xsl:text>
        <xsl:value-of select="."/>
        <xsl:text>"&gt;&#xa;</xsl:text>
    </xsl:template>

    <xsl:template match="section/section">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="section/section/title">
        <xsl:text>&lt;tt="</xsl:text>
        <xsl:value-of select="."/>
        <xsl:text>"&gt;&#xa;</xsl:text>
        <xsl:apply-templates/>
    </xsl:template>

</xsl:transform>
