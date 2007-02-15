<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiML to bbCode.
  -->
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" exclude-result-prefixes="xsl">

    <xsl:output method="text" encoding="utf-8" indent="no"/>

    <xsl:strip-space elements="*"/>

<!-- If daiml/@autotoc create the ToC as a header. Insert a note about DaiML/modification time as a footer -->
    <xsl:template match="/daiml">
        <xsl:if test="@autotoc and section">
            <xsl:text>[b]</xsl:text>
            <xsl:choose>
                <xsl:when test="@autotoc='autotoc'">
                     <xsl:text>Table of Contents</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                     <xsl:value-of select="@autotoc"/>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:text>[/b]&#xA;&#xA;</xsl:text>
            <xsl:apply-templates select="section" mode="toc"/>
        </xsl:if>
        <xsl:apply-templates/>
        <xsl:text>&#xA;[size=10][color=gray][i]</xsl:text> <xsl:text>This document was automatically generated from DaiML source.&#xA;</xsl:text> <xsl:text>Last modified: </xsl:text><xsl:value-of select="current-dateTime()"/> <xsl:text>[/i][/color][/size]</xsl:text>
    </xsl:template>

<!-- Creates the ToC, skipping those sections with @toc='exclude' -->
    <xsl:template match="section" mode="toc">
        <xsl:choose>
            <xsl:when test="@toc='exclude'">
                <xsl:apply-templates select="section" mode="toc"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>[list][*]</xsl:text>
                <xsl:value-of select="normalize-space(title)"/>
                <xsl:apply-templates select="section" mode="toc"/>
                <xsl:text>[/list]</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Section -->
    <xsl:template match="section">
        <xsl:text>[list]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/list]&#xA;</xsl:text>
    </xsl:template>

<!-- Level 1 section/title -->
    <xsl:template match="/daiml/section/title">
        <xsl:text>[size=24][b]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/b][/size]&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Level 2 section/title -->
    <xsl:template match="/daiml/section/section/title">
        <xsl:text>[size=20][b]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/b][/size]&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Level 3 section/title -->
    <xsl:template match="/daiml/section/section/section/title">
        <xsl:text>[size=16][b]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/b][/size]&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Level 4+ section/title -->
    <xsl:template match="section/section/section/section/title">
        <xsl:text>[size=12][b]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/b][/size]&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Insert two LFs after new paragraph -->
    <xsl:template match="p">
        <xsl:apply-templates/>
        <xsl:text>&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Line break -->
    <xsl:template match="br">
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

<!-- Transform anchors according to a/@href -->
    <xsl:template match="a">
        <xsl:choose>
            <xsl:when test="matches(@href, '^http://')"> <!-- Fully qualified URLs: transformed to properly marked up content = clickable links -->
                <xsl:text>[url=</xsl:text> <xsl:value-of select="@href"/> <xsl:text>]</xsl:text>
                <xsl:apply-templates/>
                <xsl:text>[/url]</xsl:text>
            </xsl:when>
            <xsl:when test="matches(@href, '^#')"> <!-- Same-document references: underlined as visual indicator but non-clickable -->
                <xsl:text>[u]</xsl:text>
                <xsl:apply-templates/>
                <xsl:text>[/u]</xsl:text>
            </xsl:when>
            <xsl:when test="matches(@href, '^file=')"> <!-- Internal forum references: prefixed to become clickable links -->
                <xsl:text>[url=http://www.daimonin.net/index.php?name=PNphpBB2&amp;</xsl:text> <xsl:value-of select="@href"/> <xsl:text>]</xsl:text>
                <xsl:apply-templates/>
                <xsl:text>[/url]</xsl:text>
            </xsl:when>
            <xsl:otherwise> <!-- Relative links: underlined as visual indicator but non-clickable -->
                <xsl:text>[u]</xsl:text>
                <xsl:apply-templates/>
                <xsl:text>[/u]</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Transform images according to img/@src -->
    <xsl:template match="img">
        <xsl:choose>
            <xsl:when test="matches(@src, '^http://')"> <!-- Fully qualified URLs: transformed to properly marked-up content -->
                <xsl:text>[img]</xsl:text> <xsl:value-of select="@src"/> <xsl:text>[/img]</xsl:text>
            </xsl:when>
            <xsl:otherwise> <!-- Relative links: red warning placeholder -->
                <xsl:text>[color=red][** INVALID IMAGE URL: </xsl:text> <xsl:value-of select="@alt"/> <xsl:text> **][/color]</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Italics -->
    <xsl:template match="em">
        <xsl:text>[i]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/i]</xsl:text>
    </xsl:template>

<!-- Bold -->
    <xsl:template match="strong">
        <xsl:text>[b]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/b]</xsl:text>
    </xsl:template>

<!-- Inline code -->
    <xsl:template match="code">
        <xsl:text>[color=green]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/color]</xsl:text>
    </xsl:template>

<!-- Definition and unordered lists -->
    <xsl:template match="dl|ul">
        <xsl:text>[list]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>[/list]</xsl:text>
    </xsl:template>

<!-- Ordered lists which may be 1,2,3 or a,b,c -->
    <xsl:template match="ol">
        <xsl:choose>
            <xsl:when test="@type = 'decimal'">
                <xsl:text>[list=1]</xsl:text>
            </xsl:when>
            <xsl:when test="@type = 'alpha'">
                <xsl:text>[list=a]</xsl:text>
            </xsl:when>
            <xsl:when test="@type = 'roman'"> <!-- in nested lists transforms to decimal or alpha, whichever was not used in the ancestor list; in all other cases defaults to alpha -->
                <xsl:if test="ancestor::ol/@type = 'decimal'">
                    <xsl:text>[list=a]</xsl:text>
                </xsl:if>
                <xsl:if test="ancestor::ol/@type = 'alpha'">
                    <xsl:text>[list=1]</xsl:text>
                </xsl:if>
            </xsl:when>
        </xsl:choose>
        <xsl:apply-templates/>
        <xsl:text>[/list]</xsl:text>
    </xsl:template>

<!-- Definitions transform to indented paragraphs -->
    <xsl:template match="dd">
        <xsl:apply-templates/>
        <xsl:text>&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Definition titles and list items transform to bulleted (or numbered in an ordered list) paragraphs -->
    <xsl:template match="dt|li">
        <xsl:text>[*]</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>&#xA;&#xA;</xsl:text>
    </xsl:template>

<!-- Tables are currently not rendered -->
    <xsl:template match="table">
        <xsl:text>&#xA;[color=red][** TABLE REMOVED **][/color]&#xA;</xsl:text>
    </xsl:template>

<!-- Code blocks -->
    <xsl:template match="blockcode">
        <xsl:text>[code]</xsl:text>
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
        <xsl:if test="matches(., '^\s')">
            <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="normalize-space(.)"/>
        <xsl:if test="matches(., '\s$')">
            <xsl:text> </xsl:text>
        </xsl:if>
    </xsl:template>

</xsl:transform>
