<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <infrastructure vesion="2">
      <xsl:for-each select="attr">
        <target name="{@name}">
          <properties>
            <xsl:for-each select="attrs/attr[@name='properties']/attrs/attr">
              <property name="{@name}"><xsl:value-of select="*/@value" /></property>
            </xsl:for-each>
          </properties>
          <containers>
            <xsl:for-each select="attrs/attr[@name='containers']/attrs/attr">
              <container name="{@name}">
                <xsl:for-each select="attrs/attr">
                  <property name="{@name}"><xsl:value-of select="*/@value" /></property>
                </xsl:for-each>
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
