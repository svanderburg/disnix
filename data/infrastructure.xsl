<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <infrastructure>
      <xsl:for-each select="attr">
        <target name="{@name}">
	  <xsl:for-each select="attrs/attr">
	    <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
	  </xsl:for-each>
	</target>
      </xsl:for-each>
    </infrastructure>
  </xsl:template>
</xsl:stylesheet>
