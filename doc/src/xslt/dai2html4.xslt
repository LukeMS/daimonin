<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiML to HTML 4.01 Strict.
  -->
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" exclude-result-prefixes="xsl">

    <xsl:output method="html" doctype-public="-//W3C//DTD HTML 4.01//EN" doctype-system="http://www.w3.org/TR/html4/strict.dtd" encoding="iso-8859-1" omit-xml-declaration="yes" indent="no"/>

<!-- Sort out the basic head and body structure of the output HTML, including:
       * any author-specified inline style or external stylesheet;
       * an auto-ToC, if specifed;
       * daiml/@title as a header; and
       * a note about DaiML/modification time as a footer -->
    <xsl:template match="/daiml">
        <html>
            <head>
                <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
                <title>
                    <xsl:value-of select="@title"/>
                </title>
                <xsl:if test="style">
                    <xsl:apply-templates select="style" mode="head"/>
                </xsl:if>
                <xsl:if test="stylesheet">
                    <xsl:apply-templates select="stylesheet" mode="head"/>
                </xsl:if>
            </head>
            <body>
                <div class="daiml-document">
                    <h1>
                        <xsl:value-of select="@title"/>
                    </h1>
                </div>
                <xsl:if test="@autotoc and section">
                    <div class="daiml-toc">
                        <h4 id="toc">
                            <xsl:choose>
                                <xsl:when test="@autotoc='autotoc'">
                                    <xsl:text>Table of Contents</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="@autotoc"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </h4>
                        <xsl:apply-templates select="section" mode="toc"/>
                    </div>
                </xsl:if>
                <xsl:apply-templates/>
                <p class="daiml-generator">
                    <xsl:text>This document was automatically generated from DaiML source</xsl:text><br/>
                    <xsl:text>Last modified: </xsl:text><xsl:value-of select="current-dateTime()"/>
                </p>
            </body>
        </html>
    </xsl:template>

<!-- Transforms style and stylesheet into HTML style and link elements and ensures that they only appear in the output HTML head -->
    <xsl:template match="style" mode="head">
        <style type="text/css">
            <xsl:apply-templates/>
        </style>
    </xsl:template>

    <xsl:template match="style"/>

    <xsl:template match="stylesheet" mode="head">
        <link href="{@href}" media="{@media}" rel="stylesheet" title="{@title}" type="text/css"/>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="stylesheet"/>

<!-- Puts a backlink to the ToC, if there is one, at the bottom of every level 1 section -->
    <xsl:template match="section">
        <div class="daiml-section" id="{if (@id) then @id else generate-id()}">
            <xsl:apply-templates/>
            <xsl:if test=".. = daiml and ../@autotoc">
                <p class="daiml-toc">
                    <a href="#toc">
                        <xsl:choose>
                            <xsl:when test="/daiml/@autotoc='autotoc'">
                                <xsl:text>Table of Contents</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="/daiml/@autotoc"/>
                            </xsl:otherwise>
                        </xsl:choose>
                    </a>
                </p>
            </xsl:if>
        </div>
    </xsl:template>

<!-- Creates the ToC, skipping those sections with @toc='exclude' -->
    <xsl:template match="section" mode="toc">
        <xsl:choose>
            <xsl:when test="@toc='exclude'">
                <xsl:apply-templates select="section" mode="toc"/>
            </xsl:when>
            <xsl:otherwise>
                <ul>
                    <li>
                        <a href="#{if (@id) then @id else generate-id()}"><xsl:value-of select="normalize-space(title)"/></a>
                        <xsl:apply-templates select="section" mode="toc"/>
                    </li>
                </ul>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Level 1 section header -->
    <xsl:template match="/daiml/section/title">
        <h2>
            <xsl:apply-templates/>
        </h2>
    </xsl:template>

<!-- Level 2 section header -->
    <xsl:template match="/daiml/section/section/title">
        <h3>
            <xsl:apply-templates/>
        </h3>
    </xsl:template>

<!-- Level 3 section header -->
    <xsl:template match="/daiml/section/section/section/title">
        <h4>
            <xsl:apply-templates/>
        </h4>
    </xsl:template>

<!-- Level 4 section header -->
    <xsl:template match="/daiml/section/section/section/section/title">
        <h5>
            <xsl:apply-templates/>
        </h5>
    </xsl:template>

<!-- Level 5+ section header -->
    <xsl:template match="section/section/section/section/section/title">
        <h6>
            <xsl:apply-templates/>
        </h6>
    </xsl:template>

<!-- Code blocks -->
    <xsl:template match="blockcode">
        <pre>
            <xsl:apply-templates/>
        </pre>
    </xsl:template>

<!-- Anchors: properly transforms a/@href -->
    <xsl:template match="a">
        <a href="{if (matches(@href, 'dai$') or matches(@href, '.dai#')) then replace(@href, '.dai', '.html') else @href}">
            <xsl:apply-templates select="@*[not(local-name()='href')]"/>
            <xsl:apply-templates/>
        </a>
    </xsl:template>

<!-- Ordered list: Gives the output element a class according to ol/@type rather than using the deprecated HTML type attribute or a specific style attribute. May change? -->
    <xsl:template match="ol">
        <ol class="daiml-{@type}">
            <xsl:apply-templates/>
        </ol>
    </xsl:template>

    <xsl:template match="*">
        <xsl:element name="{local-name()}">
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>

<!-- Tables -->
    <xsl:template match="table/@border">
        <xsl:if test=".='yes'">
            <xsl:attribute name="border">1</xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template match="@colspan[.='1']|@rowspan[.='1']"/>

    <xsl:template match="@*">
        <xsl:copy/>
    </xsl:template>

    <xsl:template match="blockcode/text()">
        <xsl:copy/>
    </xsl:template>

    <xsl:template match="text()">
        <xsl:if test="matches(.,'^\s')">
            <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="normalize-space(.)"/>
        <xsl:if test="matches(.,'\s$')">
            <xsl:text> </xsl:text>
        </xsl:if>
    </xsl:template>

</xsl:transform>
