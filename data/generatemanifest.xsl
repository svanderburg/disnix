<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <manifest>
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
	              <xsl:element name="{@name}">
			<xsl:value-of select="*/@value" />
			<xsl:for-each select="list/*">
			  <xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
			</xsl:for-each>
		      </xsl:element>
	            </xsl:for-each>
		  </target>
		</dependency>
	      </xsl:for-each>
	    </dependsOn>
	    <name><xsl:value-of select="attr[@name='name']/string/@value" /></name>
	    <service><xsl:value-of select="attr[@name='service']/string/@value" /></service>
	    <target>
	      <xsl:for-each select="attr[@name='target']/attrs/attr">
	        <xsl:element name="{@name}">
		  <xsl:value-of select="*/@value" />
		  <xsl:for-each select="list/*">
		    <xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
		  </xsl:for-each>
		</xsl:element>
	      </xsl:for-each>
	    </target>
	    <targetProperty><xsl:value-of select="attr[@name='targetProperty']/string/@value" /></targetProperty>
	    <type><xsl:value-of select="attr[@name='type']/string/@value" /></type>
	  </mapping>
        </xsl:for-each>
      </activation>
      <targets>
	<xsl:for-each select="attr[@name='targets']/list/attrs">
	  <target>
	    <targetProperty><xsl:value-of select="attr[@name='targetProperty']/string/@value" /></targetProperty>
	    <numOfCores><xsl:value-of select="attr[@name='numOfCores']/int/@value" /></numOfCores>
	  </target>
	</xsl:for-each>
      </targets>
    </manifest>
  </xsl:template>
</xsl:stylesheet>
