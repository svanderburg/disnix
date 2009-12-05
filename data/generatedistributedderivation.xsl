<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/list">
    <distributedderivation>
      <xsl:for-each select="attrs">
        <mapping>
          <derivation><xsl:value-of select="attr[@name='derivation']/string/@value" /></derivation>
	  <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
	</mapping>
      </xsl:for-each>
    </distributedderivation>
  </xsl:template>
</xsl:stylesheet>
