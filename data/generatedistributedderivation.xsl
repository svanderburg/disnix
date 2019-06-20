<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <distributedderivation version="2">
      <derivationMappings>
        <xsl:for-each select="attr[@name='derivationMappings']/list/attrs">
          <mapping>
            <derivation><xsl:value-of select="attr[@name='derivation']/*/@value" /></derivation>
            <interface><xsl:value-of select="attr[@name='interface']/*/@value" /></interface>
          </mapping>
        </xsl:for-each>
      </derivationMappings>
      <interfaces>
        <xsl:for-each select="attr[@name='interfaces']/attrs/attr">
          <interface name="{@name}">
            <targetAddress><xsl:value-of select="attrs/attr[@name='targetAddress']/*/@value" /></targetAddress>
            <clientInterface><xsl:value-of select="attrs/attr[@name='clientInterface']/*/@value" /></clientInterface>
          </interface>
        </xsl:for-each>
      </interfaces>
    </distributedderivation>
  </xsl:template>
</xsl:stylesheet>
