<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <!-- Transformation templates -->

  <xsl:template match="string | int | float | bool">
    <xsl:attribute name="type">
      <xsl:value-of select="local-name()" />
    </xsl:attribute>

    <xsl:value-of select="@value" />
  </xsl:template>

  <xsl:template match="list">
    <xsl:attribute name="type">list</xsl:attribute>

    <xsl:for-each select="*">
      <elem>
        <xsl:apply-templates select="." />
      </elem>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="convert_attrs_verbose">
    <xsl:for-each select="attr">
      <property name="{@name}">
        <xsl:apply-templates select="*" />
      </property>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="attrs">
    <xsl:attribute name="type">attrs</xsl:attribute>
    <xsl:call-template name="convert_attrs_verbose" />
  </xsl:template>

  <!-- Transformation procedure -->

  <xsl:template match="/expr/attrs">
    <infrastructure version="2">
      <xsl:for-each select="attr">
        <target name="{@name}">
          <properties>
            <xsl:apply-templates select="attrs/attr[@name='properties']/*" />
          </properties>
          <containers>
            <xsl:for-each select="attrs/attr[@name='containers']/attrs/attr">
              <container name="{@name}">
                <xsl:apply-templates select="*" />
              </container>
            </xsl:for-each>
          </containers>

          <system><xsl:value-of select="attrs/attr[@name='system']/*/@value" /></system>
          <numOfCores><xsl:value-of select="attrs/attr[@name='numOfCores']/*/@value" /></numOfCores>
          <clientInterface><xsl:value-of select="attrs/attr[@name='clientInterface']/*/@value" /></clientInterface>
          <targetProperty><xsl:value-of select="attrs/attr[@name='targetProperty']/*/@value" /></targetProperty>
        </target>
      </xsl:for-each>
    </infrastructure>
  </xsl:template>
</xsl:stylesheet>
