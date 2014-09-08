Feature: Validate a blueprint

  Scenario: Validate a valid blueprint file

    When I run `snowcrash --validate blueprint.apib`
    Then the output should contain:
    """
    OK
    """

  Scenario: Validate a valid blueprint input

    When I run `snowcrash --validate` interactively
    When I pipe in the file "blueprint.apib"
    Then the output should contain:
    """
    OK
    """

  Scenario: Validate an invalid blueprint file

    When I run `snowcrash --validate invalid_blueprint.apib`
    Then the output should contain:
    """
    OK.
    warning: (5)  unexpected header block, expected a group, resource or an action definition, e.g. '# Group <name>', '# <resource name> [<URI>]' or '# <HTTP method> <URI>' :24:29
    """

  Scenario: Validate an invalid blueprint input

    When I run `snowcrash --validate` interactively
    When I pipe in the file "invalid_blueprint.apib"
    Then the output should contain:
    """
    OK.
    warning: (5)  unexpected header block, expected a group, resource or an action definition, e.g. '# Group <name>', '# <resource name> [<URI>]' or '# <HTTP method> <URI>' :24:29
    """
