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
        doctype-public="-//W3C//DTD XHTML 1.1//EN"
        doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
        encoding="utf-8"
        indent="no"
    />

    <xsl:template match="/">
        <html>
            <head>
                <title><xsl:apply-templates select="section/title/text()"/></title>
                <!-- Reasons for inlining the stylesheet instead of referencing an external file:
                  - The link to the external file would need to be relative, but what about html files in subdirectories?
                  -->
                <style type="text/css">
                    h2 {
                        border-top-style:solid;
                        border-top-width:1px;
                        border-top-color:#000;
                        background-color:#eee;
                    }
                    div.section {
                        margin-left:1em;
                    }
                    div.section h2, div.section h3, div.section h4 {
                        margin-left:-1em;
                    }
                </style>
            </head>
            <body>
                <xsl:apply-templates/>
                <p>
                    <xsl:text>Last modified: </xsl:text> <xsl:value-of select="current-dateTime()"/>
                </p>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="/section">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="/section/title">
        <h1>
            <xsl:if test="../@id"><xsl:attribute name="id" select="../@id"/></xsl:if>
            <xsl:apply-templates/>
        </h1>
        <xsl:if test="/section/@autotoc">
            <h2 id="toc">
                <xsl:choose>
                    <xsl:when test="/section/@autotoc='autotoc'">
                        <xsl:text>Table of Contents</xsl:text>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="/section/@autotoc"/>
                    </xsl:otherwise>
                </xsl:choose>
            </h2>
            <xsl:apply-templates select="/section/section" mode="toc"/>
        </xsl:if>
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
                    </li>
                    <xsl:apply-templates select="section" mode="toc" />
                </ul>
            </xsl:otherwise>
        </xsl:choose>
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

    <xsl:template match="/section/section">
        <div class="section" id="{if (@id) then @id else generate-id()}">
            <xsl:apply-templates/>
        </div>
        <xsl:if test="/section/@autotoc">
            <p>
                <a href="#toc">
                    <xsl:choose>
                        <xsl:when test="/section/@autotoc='autotoc'">
                            <xsl:text>Table of Contents</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="/section/@autotoc"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </a>
            </p>
        </xsl:if>
    </xsl:template>

    <xsl:template match="section/section/section">
        <div class="section" id="{if (@id) then @id else generate-id()}">
            <xsl:apply-templates/>
        </div>
    </xsl:template>

    <xsl:template match="a">
        <a href="{replace(@href, '.dai$', '.xhtml')}">
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
        <xsl:if test="matches(.,'^\s')"><xsl:text> </xsl:text></xsl:if>
        <xsl:value-of select="normalize-space(.)"/>
        <xsl:if test="matches(.,'\s$')"><xsl:text> </xsl:text></xsl:if>
    </xsl:template>

</xsl:transform>
