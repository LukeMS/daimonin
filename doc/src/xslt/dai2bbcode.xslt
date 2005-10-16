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

    <xsl:preserve-space elements="blockcode" />
    <xsl:strip-space elements="*" />

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
        <xsl:text>&#xA;</xsl:text>
        <xsl:text>Last modified: </xsl:text>
        <xsl:value-of select="current-dateTime()"/>
    </xsl:template>

    <xsl:template match="/section/title">
        <xsl:text>[size=24]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/size]&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="/section/section/title">
        <xsl:text>[size=20]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/size]&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="/section/section/section/title">
        <xsl:text>[size=16]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/size]&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="/section/section/section/section/title">
        <xsl:text>[size=14]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/size]&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="blockcode">
        <xsl:text>[code]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/code]</xsl:text>
    </xsl:template>

    <xsl:template match="em">
        <xsl:text>[i]</xsl:text><xsl:apply-templates/><xsl:text>[/i]</xsl:text>
    </xsl:template>

    <xsl:template match="strong">
        <xsl:text>[b]</xsl:text><xsl:apply-templates/><xsl:text>[/b]</xsl:text>
    </xsl:template>

    <xsl:template match="code">
        <xsl:text>[b]</xsl:text><xsl:apply-templates/><xsl:text>[/b]</xsl:text>
    </xsl:template>

    <xsl:template match="ul">
        <xsl:text>[list]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/list]</xsl:text>
    </xsl:template>

    <xsl:template match="ol">
        <xsl:text>[list=1]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/list]</xsl:text>
    </xsl:template>

    <xsl:template match="li">
        <xsl:text>[*]</xsl:text><xsl:apply-templates/><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="dl/dt">
        <xsl:text>[b]</xsl:text><xsl:apply-templates/><xsl:text>[/b]</xsl:text><xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="dl/dt/code|dl/dt/strong">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="dl/dd">
        <xsl:text>    </xsl:text><xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="p|section">
        <xsl:apply-templates/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="a">
        <xsl:text>[url=</xsl:text><xsl:value-of select="@href"/><xsl:text>]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/url]</xsl:text>
    </xsl:template>

    <xsl:template match="br">
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="*">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="blockcode/text()">
        <xsl:value-of select="."/>
    </xsl:template>

    <xsl:template match="text()">
        <xsl:if test="matches(.,'^\s')"><xsl:text> </xsl:text></xsl:if>
        <xsl:value-of select="normalize-space(.)"/>
        <xsl:if test="matches(.,'\s$')"><xsl:text> </xsl:text></xsl:if>
    </xsl:template>

</xsl:transform>
