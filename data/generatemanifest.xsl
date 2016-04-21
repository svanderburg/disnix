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
		  <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
		  <key><xsl:value-of select="attr[@name='_key']/string/@value" /></key>
		</dependency>
	      </xsl:for-each>
	    </dependsOn>
	    <name><xsl:value-of select="attr[@name='name']/string/@value" /></name>
	    <service><xsl:value-of select="attr[@name='service']/string/@value" /></service>
	    <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
	    <type><xsl:value-of select="attr[@name='type']/string/@value" /></type>
	    <key><xsl:value-of select="attr[@name='_key']/string/@value" /></key>
	  </mapping>
        </xsl:for-each>
      </activation>
      <snapshots>
	<xsl:for-each select="attr[@name='snapshots']/list/attrs">
	  <mapping>
	    <component><xsl:value-of select="attr[@name='component']/string/@value" /></component>
	    <container><xsl:value-of select="attr[@name='container']/string/@value" /></container>
	    <target><xsl:value-of select="attr[@name='target']/string/@value" /></target>
	    <service><xsl:value-of select="attr[@name='service']/string/@value" /></service>
	  </mapping>
	</xsl:for-each>
      </snapshots>
      <targets>
	<xsl:for-each select="attr[@name='targets']/list/attrs">
	  <target>
	    <xsl:for-each select="attr[@name='properties']/attrs">
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
	    
	    <xsl:for-each select="attr[@name='containers']/attrs">
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

	    <system><xsl:value-of select="attr[@name='system']/string/@value" /></system>
	    <numOfCores><xsl:value-of select="attr[@name='numOfCores']/*/@value" /></numOfCores>
	    <clientInterface><xsl:value-of select="attr[@name='clientInterface']/string/@value" /></clientInterface>
	    <targetProperty><xsl:value-of select="attr[@name='targetProperty']/string/@value" /></targetProperty>
	  </target>
	</xsl:for-each>
      </targets>
    </manifest>
  </xsl:template>
</xsl:stylesheet>
