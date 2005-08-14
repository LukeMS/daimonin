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

    <xsl:strip-space elements="*" />
    <xsl:preserve-space elements="blockcode" />

    <xsl:output
        method="xml"
        doctype-public="-//W3C//DTD HTML 4.01//EN"
        doctype-system="http://www.w3.org/TR/html4/strict.dtd"
        encoding="iso-8859-1"
        omit-xml-declaration="yes"
        indent="no"
    />

    <xsl:template match="/">
        <html>
            <head>
                <title><xsl:apply-templates select="section/title/text()"/></title>
            </head>
            <body>
                <xsl:apply-templates/>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="/section/title">
        <h1>
            <xsl:if test="../@id"><xsl:attribute name="id" select="../@id"/></xsl:if>
            <xsl:apply-templates/>
        </h1>
    </xsl:template>

    <xsl:template match="/section/section/title">
        <h2>
            <xsl:if test="../@id"><xsl:attribute name="id" select="../@id"/></xsl:if>
            <xsl:apply-templates/>
        </h2>
    </xsl:template>

    <xsl:template match="/section/section/section/title">
        <h3>
            <xsl:if test="../@id"><xsl:attribute name="id" select="../@id"/></xsl:if>
            <xsl:apply-templates/>
        </h3>
    </xsl:template>

    <xsl:template match="/section/section/section/section/title">
        <h4>
            <xsl:if test="../@id"><xsl:attribute name="id" select="../@id"/></xsl:if>
            <xsl:apply-templates/>
        </h4>
    </xsl:template>

    <xsl:template match="blockcode">
        <pre>
            <xsl:apply-templates/>
        </pre>
    </xsl:template>

    <xsl:template match="section">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="*">
        <xsl:element name="{local-name()}">
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>

    <xsl:template match="@*">
        <xsl:copy/>
    </xsl:template>

</xsl:transform>
