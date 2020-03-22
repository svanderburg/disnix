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

  <xsl:template match="/expr/attrs">
    <profilemanifest version="2">
      <services>
        <xsl:for-each select="attr[@name='services']/attrs/attr">
          <service name="{@name}">
            <name><xsl:value-of select="attrs/attr[@name='name']/*/@value" /></name>
            <pkg><xsl:value-of select="attrs/attr[@name='pkg']/*/@value" /></pkg>
            <type><xsl:value-of select="attrs/attr[@name='type']/*/@value" /></type>
            <dependsOn>
              <xsl:for-each select="attrs/attr[@name='dependsOn']/list/attrs">
                <mapping>
                  <xsl:for-each select="attr">
                    <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
                  </xsl:for-each>
                </mapping>
              </xsl:for-each>
            </dependsOn>
            <connectsTo>
              <xsl:for-each select="attrs/attr[@name='connectsTo']/list/attrs">
                <mapping>
                  <xsl:for-each select="attr">
                    <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
                  </xsl:for-each>
                </mapping>
              </xsl:for-each>
            </connectsTo>
            <providesContainers>
              <xsl:for-each select="attrs/attr[@name='providesContainers']/attrs/attr">
                <container name="{@name}">
                  <xsl:apply-templates select="*" />
                </container>
              </xsl:for-each>
            </providesContainers>
          </service>
        </xsl:for-each>
      </services>

      <serviceMappings>
        <xsl:for-each select="attr[@name='serviceMappings']/list/attrs">
          <mapping>
            <xsl:for-each select="attr">
              <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
            </xsl:for-each>
          </mapping>
        </xsl:for-each>
      </serviceMappings>

      <snapshotMappings>
        <xsl:for-each select="attr[@name='snapshotMappings']/list/attrs">
          <mapping>
            <xsl:for-each select="attr">
              <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
            </xsl:for-each>
          </mapping>
        </xsl:for-each>
      </snapshotMappings>
    </profilemanifest>
  </xsl:template>
</xsl:stylesheet>
