<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <distributionexport>
      <distribution>
        <xsl:for-each select="attr[@name='profiles']/list/attrs">
	  <mapping>
	    <profile><xsl:value-of select="attr[@name='profile']/string/@value" /></profile>
	    <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
	  </mapping>
        </xsl:for-each>
      </distribution>
      <activation>
        <xsl:for-each select="attr[@name='activation']/list/attrs">
	  <mapping>
            <dependsOn>
	      <xsl:for-each select="attr[@name='dependsOn']/list/attrs">
	        <dependency>
	          <service><xsl:value-of select="attr[@name='service']/string/@value" /></service>
		  <target>
		    <xsl:for-each select="attr[@name='target']/attrs/attr">
	              <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
	            </xsl:for-each>
		  </target>
		</dependency>
	      </xsl:for-each>
	    </dependsOn>
	    <service><xsl:value-of select="attr[@name='service']/string/@value" /></service>
	    <target>
	      <xsl:for-each select="attr[@name='target']/attrs/attr">
	        <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
	      </xsl:for-each>
	    </target>
	    <targetProperty><xsl:value-of select="attr[@name='targetProperty']/string/@value" /></targetProperty>
	    <type><xsl:value-of select="attr[@name='type']/string/@value" /></type>
	  </mapping>
        </xsl:for-each>
      </activation>
    </distributionexport>
  </xsl:template>
</xsl:stylesheet>
