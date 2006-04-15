<?xml version="1.0" encoding="utf-8"?>
<!--
  - Transformation for converting DaiML to plain XHTML 1.1.
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
        doctype-public="-//W3C//DTD XHTML 1.1//EN"
        doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
        encoding="utf-8"
        indent="no"
    />

    <xsl:template match="/daiml">
        <html>
            <head>
                <title>
                    <xsl:value-of select="@title"/>
                </title>
                <xsl:choose>
                    <xsl:when test="stylesheet">
                        <xsl:apply-templates select="stylesheet"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <style type="text/css">
h2
{
    background-color:#eee;
    border-top:1px solid #000;
}

div.section
{
    margin-left:1em;
}

div.section h2, div.section h3, div.section h4, div.section h5, div.section h6
{
    margin-left:-1em;
}
                        </style>
                    </xsl:otherwise>
                </xsl:choose>
            </head>
            <body>
                <div class="document">
                    <h1>
                        <xsl:value-of select="@title"/>
                    </h1>
                </div>
                <xsl:if test="@autotoc and section">
                    <div class="toc">
                        <h2 id="toc">
                            <xsl:choose>
                                <xsl:when test="@autotoc='autotoc'">
                                    <xsl:text>Table of Contents</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="@autotoc"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </h2>
                        <xsl:apply-templates select="section" mode="toc"/>
                    </div>
                </xsl:if>
                <xsl:apply-templates/>
                <p>
                    <xsl:text>Last modified: </xsl:text>
                    <xsl:value-of select="current-dateTime()"/>
                </p>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="stylesheet">
        <link href="{@href}" media="{@media}" rel="stylesheet" title="{@title}" type="text/css"/>
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="/daiml/section">
        <div class="section" id="{if (@id) then @id else generate-id()}">
            <xsl:apply-templates/>
            <xsl:if test="/daiml/@autotoc">
                <p class="toc">
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

    <xsl:template match="section/section">
        <div class="section" id="{if (@id) then @id else generate-id()}">
            <xsl:apply-templates/>
        </div>
    </xsl:template>

    <xsl:template match="section" mode="toc">
        <xsl:choose>
            <xsl:when test="@toc='exclude'">
                <xsl:apply-templates select="section" mode="toc"/>
            </xsl:when>
            <xsl:otherwise>
                <ul>
                    <li>
                        <a href="#{if (@id) then @id else generate-id()}"><xsl:apply-templates select="title/node()"/></a>
                        <xsl:apply-templates select="section" mode="toc"/>
                    </li>
                </ul>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="/daiml/section/title">
        <h2>
            <xsl:apply-templates/>
        </h2>
    </xsl:template>

    <xsl:template match="/daiml/section/section/title">
        <h3>
            <xsl:apply-templates/>
        </h3>
    </xsl:template>

    <xsl:template match="/daiml/section/section/section/title">
        <h4>
            <xsl:apply-templates/>
        </h4>
    </xsl:template>

    <xsl:template match="/daiml/section/section/section/section/title">
        <h5>
            <xsl:apply-templates/>
        </h5>
    </xsl:template>

    <xsl:template match="section/section/section/section/section/title">
        <h6>
            <xsl:apply-templates/>
        </h6>
    </xsl:template>

    <xsl:template match="blockcode">
        <pre>
            <xsl:apply-templates/>
        </pre>
    </xsl:template>


    <xsl:template match="a">
        <a href="{if (matches(@href, 'dai$') or matches(@href, '.dai#')) then replace(@href, '.dai', '.xhtml') else @href}">
            <xsl:apply-templates select="@*[not(local-name()='href')]"/>
            <xsl:apply-templates/>
        </a>
    </xsl:template>

    <xsl:template match="*">
        <xsl:element name="{local-name()}">
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>

    <xsl:template match="table/@border">
        <xsl:if test=".='yes'">
            <xsl:attribute name="border">border</xsl:attribute>
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
