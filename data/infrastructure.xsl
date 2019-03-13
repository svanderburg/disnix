<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <infrastructure version="1">
      <xsl:for-each select="attr">
	<target>
	  <name><xsl:value-of select="@name" /></name>

	  <xsl:for-each select="attrs/attr[@name='properties']/attrs">
	    <properties>
	      <xsl:for-each select="attr">
	        <xsl:element name="{@name}">
		  <xsl:value-of select="*/@value" />
		  <xsl:for-each select="list/*">
		    <xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
		  </xsl:for-each>
		</xsl:element>
	      </xsl:for-each>
	    </properties>
	  </xsl:for-each>
	
	  <xsl:for-each select="attrs/attr[@name='containers']/attrs">
	    <containers>
	      <xsl:for-each select="attr">
		<xsl:element name="{@name}">
		  <xsl:for-each select="attrs/attr">
		    <xsl:element name="{@name}">
		      <xsl:value-of select="*/@value" />
		      <xsl:for-each select="list/*">
		        <xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
		      </xsl:for-each>
		    </xsl:element>
		  </xsl:for-each>
		</xsl:element>
	      </xsl:for-each>
	    </containers>
	  </xsl:for-each>

	  <system><xsl:value-of select="attrs/attr[@name='system']/string/@value" /></system>
	  <numOfCores><xsl:value-of select="attrs/attr[@name='numOfCores']/*/@value" /></numOfCores>
	  <clientInterface><xsl:value-of select="attrs/attr[@name='clientInterface']/string/@value" /></clientInterface>
	  <targetProperty><xsl:value-of select="attrs/attr[@name='targetProperty']/string/@value" /></targetProperty>
	</target>
      </xsl:for-each>
    </infrastructure>
  </xsl:template>
</xsl:stylesheet>
