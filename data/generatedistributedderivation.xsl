<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <distributedderivation>
      <build>
        <xsl:for-each select="attr[@name='build']/list/attrs">
          <mapping>
            <derivation><xsl:value-of select="attr[@name='derivation']/string/@value" /></derivation>
            <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
          </mapping>
        </xsl:for-each>
      </build>
      <interfaces>
        <xsl:for-each select="attr[@name='interfaces']/list/attrs">
          <interface>
            <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
            <clientInterface><xsl:value-of select="attr[@name='clientInterface']/string/@value" /></clientInterface>
          </interface>
        </xsl:for-each>
      </interfaces>
    </distributedderivation>
  </xsl:template>
</xsl:stylesheet>
