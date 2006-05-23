<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiDoc to BBCode.
  -->
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" exclude-result-prefixes="xsl">

    <xsl:strip-space elements="*"/>
    <xsl:preserve-space elements="blockcode"/>

    <xsl:output method="text" encoding="utf-8" indent="no"/>

    <xsl:template match="/daiml">
        <xsl:text>[size=20][b]</xsl:text>
            <xsl:value-of select="@title"/>
        <xsl:text>[/b][/size]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>&#xA;&#xA;&#xA;[size=10][color=gray][i]</xsl:text>
            <xsl:text>This document was automatically generated from DaiML source.&#xA;</xsl:text>
            <xsl:text>Last modified: </xsl:text><xsl:value-of select="current-dateTime()"/>
        <xsl:text>[/i][/color][/size]</xsl:text>
    </xsl:template>

    <xsl:template match="section">
        <xsl:text>&#xA;</xsl:text>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="/daiml/section/title">
        <xsl:text>&#xA;&#xA;[size=18][b]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/b][/size]</xsl:text>
    </xsl:template>

    <xsl:template match="/daiml/section/section/title">
        <xsl:text>&#xA;&#xA;[size=16][b]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/b][/size]</xsl:text>
    </xsl:template>

    <xsl:template match="/daiml/section/section/section/title">
        <xsl:text>&#xA;&#xA;[size=14][b]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/b][/size]</xsl:text>
    </xsl:template>

    <xsl:template match="section/section/section/section/title">
        <xsl:text>&#xA;&#xA;[size=12][b]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/b][/size]</xsl:text>
    </xsl:template>

    <xsl:template match="p">
        <xsl:text>&#xA;&#xA;</xsl:text>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="br">
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="a">
        <xsl:choose>
            <xsl:when test="matches(@href, '^http://')">
                <xsl:text>[url=</xsl:text><xsl:value-of select="@href"/><xsl:text>]</xsl:text>
                    <xsl:apply-templates/>
                <xsl:text>[/url]</xsl:text>
            </xsl:when>
            <xsl:when test="matches(@href, '^#')">
                <xsl:text>[color=blue]</xsl:text>
                    <xsl:apply-templates/>
                <xsl:text>[/color]</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="em">
        <xsl:text>[i]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/i]</xsl:text>
    </xsl:template>

    <xsl:template match="strong">
        <xsl:text>[b]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/b]</xsl:text>
    </xsl:template>

    <xsl:template match="code">
        <xsl:text>[b]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/b]</xsl:text>
    </xsl:template>

    <xsl:template match="dl|ol|ul">
        <xsl:choose>
            <xsl:when test="@type='decimal'">
                <xsl:text>&#xA;&#xA;[list=1]</xsl:text>
            </xsl:when>
            <xsl:when test="@type='roman'">
                <xsl:text>&#xA;&#xA;[list=1]</xsl:text>
            </xsl:when>
            <xsl:when test="@type='alpha'">
                <xsl:text>&#xA;&#xA;[list=a]</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>&#xA;&#xA;[list]</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
            <xsl:apply-templates/>
        <xsl:text>&#xA;[/list]</xsl:text>
    </xsl:template>

    <xsl:template match="dd">
        <xsl:text>&#xA;&#xA;</xsl:text>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="dt">
        <xsl:text>&#xA;&#xA;[*][b][i]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/i][/b]</xsl:text>
    </xsl:template>

    <xsl:template match="li">
        <xsl:text>&#xA;&#xA;[*]</xsl:text>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="blockcode">
        <xsl:text>&#xA;&#xA;[code]</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>[/code]</xsl:text>
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
